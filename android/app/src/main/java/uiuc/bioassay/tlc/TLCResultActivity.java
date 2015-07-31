package uiuc.bioassay.tlc;

import android.app.ActionBar;
import android.content.Intent;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.InputFilter;
import android.util.Log;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import uiuc.bioassay.tlc.camera.CameraActivity;
import uiuc.bioassay.tlc.proc.TLCProcActivity;

import static uiuc.bioassay.tlc.TLCApplication.round;


public class TLCResultActivity extends AppCompatActivity {

    private static final int COLUMN_WIDTH = 96;
    private static final int RF_COLUMN_WIDTH = 36;
    private static final int D_COLUMN_WIDTH = COLUMN_WIDTH - RF_COLUMN_WIDTH;

    private static final int GET_DATA_REQUEST = 1;
    private static final int PROCESS_DATA_REQUEST = 2;

    private int numConcs;
    private TableLayout tableLayout;
    private int currentIdx = 1;
    private String rootFolder;
    private String currFolder;
    double[] Rf;
    double[] D;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_tlcresult);
        Intent intent = getIntent();
        rootFolder = intent.getStringExtra(TLCApplication.FOLDER_EXTRA);
        numConcs = intent.getIntExtra(TLCApplication.NUM_CONCS, 3);

        tableLayout = (TableLayout) findViewById(R.id.result_table);
        TableLayout.LayoutParams lp = new TableLayout.LayoutParams(TableLayout.LayoutParams.MATCH_PARENT, TableLayout.LayoutParams.MATCH_PARENT);

        // Initialize header column
        TableRow headerRow = new TableRow(this);
        headerRow.setLayoutParams(lp);
        for (int i = 0; i < numConcs; ++i) {
            EditText editText = new EditText(this);
            InputFilter[] filterArray = new InputFilter[1];
            filterArray[0] = new InputFilter.LengthFilter(7);
            editText.setFilters(filterArray);
            editText.setMinimumWidth(COLUMN_WIDTH);
            editText.setInputType(2);
            headerRow.addView(editText);
        }
        tableLayout.addView(headerRow);

        // Initialize concentration type row
        TableRow typeConcRow = new TableRow(this);
        typeConcRow.setLayoutParams(lp);
        for (int i = 0; i < numConcs; ++i) {
            Spinner spinner = new Spinner(this);
            spinner.setMinimumWidth(COLUMN_WIDTH);
            ArrayAdapter<CharSequence> arrayAdapter = ArrayAdapter
                    .createFromResource(this, R.array.concs_type, android.R.layout.simple_spinner_item);
            arrayAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            spinner.setAdapter(arrayAdapter);
            typeConcRow.addView(spinner);
        }
        tableLayout.addView(typeConcRow);

        // Initialize title row
        TableRow titleRow = new TableRow(this);
        titleRow.setLayoutParams(lp);
        for (int i = 0; i < numConcs; ++i) {
            LinearLayout linearLayout = new LinearLayout(this);
            linearLayout.setOrientation(LinearLayout.HORIZONTAL);

            linearLayout.setMinimumWidth(COLUMN_WIDTH);
            TextView textViewRF = new TextView(this);
            textViewRF.setGravity(17);
            textViewRF.setMinimumWidth(RF_COLUMN_WIDTH);
            textViewRF.setText("Rf");
            linearLayout.addView(textViewRF);

            TextView textViewD = new TextView(this);
            textViewD.setGravity(17);
            textViewD.setMinimumWidth(D_COLUMN_WIDTH);
            textViewD.setText("D");
            linearLayout.addView(textViewD);
            titleRow.addView(linearLayout);
        }
        tableLayout.addView(titleRow);

        // Initialize average layout
        LinearLayout avgLinearLayout = (LinearLayout) findViewById(R.id.avg_linear_layout);
        for (int i = 0; i < numConcs; ++i) {
            LinearLayout linearLayout = new LinearLayout(TLCResultActivity.this);
            linearLayout.setOrientation(LinearLayout.HORIZONTAL);

            linearLayout.setMinimumWidth(COLUMN_WIDTH);
            TextView textViewRF = new TextView(TLCResultActivity.this);
            textViewRF.setGravity(17);
            textViewRF.setMinimumWidth(RF_COLUMN_WIDTH);
            textViewRF.setText("" + 0);
            linearLayout.addView(textViewRF);

            TextView textViewD = new TextView(TLCResultActivity.this);
            textViewD.setGravity(17);
            textViewD.setMinimumWidth(D_COLUMN_WIDTH);
            textViewD.setText("" + 0);
            linearLayout.addView(textViewD);
            avgLinearLayout.addView(linearLayout);
        }

        Rf = new double[numConcs];
        D = new double[numConcs];
        // Initialize Rf and D
        for (int i = 0; i < numConcs; ++i) {
            Rf[i] = 0;
            D[i] = 0;
        }

        Button getDataButton = (Button) findViewById(R.id.get_data);
        getDataButton.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        currFolder = rootFolder + File.separator + currentIdx;
                        Intent intent = new Intent(TLCResultActivity.this, CameraActivity.class);
                        intent.putExtra(TLCApplication.FOLDER_EXTRA, currFolder);
                        startActivityForResult(intent, GET_DATA_REQUEST);
                    }
                }
        );

        Button calcButton = (Button) findViewById(R.id.calculate);
        calcButton.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        List<Integer> stdConcs = new ArrayList<>();
                        List<Integer> unknownConcs = new ArrayList<>();

                        List<Double> xs = new ArrayList<>();
                        List<Double> ys = new ArrayList<>();
                        List<Double> yUnknown = new ArrayList<>();
                        TableRow concTypeRow = (TableRow) tableLayout.getChildAt(1);
                        LinearLayout avgLinearLayout = (LinearLayout) findViewById(R.id.avg_linear_layout);

                        for (int i = 0; i < numConcs; ++i) {
                            Spinner spinner = (Spinner) concTypeRow.getChildAt(i);
                            if (spinner.getSelectedItem().toString().equals("std")) {
                                stdConcs.add(i);
                                TableRow tr = (TableRow) tableLayout.getChildAt(0);
                                EditText editText = (EditText) tr.getChildAt(i);
                                String conc = editText.getText().toString();
                                if (conc.equals("")) {
                                    Toast.makeText(TLCResultActivity.this, "Empty standard concentration", Toast.LENGTH_SHORT).show();
                                    return;
                                }
                                xs.add(Double.parseDouble(conc));
                                LinearLayout linearLayout = (LinearLayout) avgLinearLayout.getChildAt(i);
                                TextView textView = (TextView) linearLayout.getChildAt(1);
                                ys.add(Double.parseDouble(textView.getText().toString()));
                            } else {
                                unknownConcs.add(i);
                                LinearLayout linearLayout = (LinearLayout) avgLinearLayout.getChildAt(i);
                                TextView textView = (TextView) linearLayout.getChildAt(1);
                                yUnknown.add(Double.parseDouble(textView.getText().toString()));
                            }
                        }
                        if (stdConcs.size() < 2) {
                            Toast.makeText(TLCResultActivity.this, "Number of standards is less than 2", Toast.LENGTH_SHORT).show();
                            return;
                        }
                        if (unknownConcs.size() == 0) {
                            Toast.makeText(TLCResultActivity.this, "No unknown to predict", Toast.LENGTH_SHORT).show();
                            return;
                        }

                        double a1 = 0;
                        double a2 = 0;
                        double c1 = 0;
                        double c2 = 0;
                        for (int i = 0; i < stdConcs.size(); ++i) {
                            double x = xs.get(i);
                            double y = ys.get(i);
                            a1 += x;
                            a2 += x * x;
                            c1 += y;
                            c2 += x * y;
                        }
                        double b1 = 2;
                        double b2 = a1;
                        double a = (b2 * c1 - b1 * c2) / (b2 * a1 - b1 * a2);
                        double b = (a2 * c1 - a1 * c2) / (a2 * b1 - a1 * b2);
                        Log.d("xxx", a1 + ", " + b1 + ", " + a2 + ", " + b2 + ", " + c1 + ", " + c2);
                        for (int i = 0; i < unknownConcs.size(); ++i) {
                            TableRow tr = (TableRow) tableLayout.getChildAt(0);
                            EditText editText = (EditText) tr.getChildAt(unknownConcs.get(i));
                            Log.d("xxx", yUnknown.get(i) + "");
                            editText.setText(Double.toString(round((yUnknown.get(i) - b)/a, 2)));
                        }
                    }
                }
        );
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // Check which request we're responding to
        if (requestCode == GET_DATA_REQUEST) {
            // Make sure the request was successful
            if (resultCode == RESULT_OK) {
                Intent intent = new Intent(TLCResultActivity.this, TLCProcActivity.class);
                intent.putExtra(TLCApplication.FOLDER_EXTRA, currFolder);
                startActivityForResult(intent, PROCESS_DATA_REQUEST);
            }
        } else if (requestCode == PROCESS_DATA_REQUEST) {
            if (resultCode == RESULT_OK) {
                double[] res = data.getDoubleArrayExtra(TLCApplication.DATA);
                setData(res);
                for (int i = 0; i < numConcs; ++i) {
                    Rf[i] += res[2*i];
                    D[i] += res[2*i + 1];
                }
                LinearLayout avgLinearLayout = (LinearLayout) findViewById(R.id.avg_linear_layout);
                for (int i = 0; i < numConcs; ++i) {
                    LinearLayout linearLayout = (LinearLayout) avgLinearLayout.getChildAt(i);
                    TextView textViewRF = (TextView) linearLayout.getChildAt(0);
                    textViewRF.setText(Double.toString(round(Rf[i] / currentIdx, 2)));

                    TextView textViewD = (TextView) linearLayout.getChildAt(1);
                    textViewD.setText(Double.toString(round(D[i] / currentIdx, 2)));
                }
                ++currentIdx;
            }
        }
    }

    private void setData(double[] data) {
        TableRow dataRow = new TableRow(this);
        TableLayout.LayoutParams lp = new TableLayout.LayoutParams(TableLayout.LayoutParams.MATCH_PARENT, TableLayout.LayoutParams.MATCH_PARENT);
        dataRow.setLayoutParams(lp);
        for (int i = 0; i < numConcs; ++i) {
            LinearLayout linearLayout = new LinearLayout(this);
            linearLayout.setOrientation(LinearLayout.HORIZONTAL);

            linearLayout.setMinimumWidth(COLUMN_WIDTH);
            TextView textView1 = new TextView(this);
            textView1.setGravity(17);
            textView1.setMinimumWidth(RF_COLUMN_WIDTH);
            textView1.setText(Double.toString(round(data[2*i], 2)));
            linearLayout.addView(textView1);

            TextView textView2 = new TextView(this);
            textView2.setGravity(17);
            textView2.setMinimumWidth(D_COLUMN_WIDTH);
            textView2.setText(Double.toString(round(data[2*i + 1], 2)));
            linearLayout.addView(textView2);
            dataRow.addView(linearLayout);
        }
        tableLayout.addView(dataRow);
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_tlcresult, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
