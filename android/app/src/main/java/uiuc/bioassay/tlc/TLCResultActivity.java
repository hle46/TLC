package uiuc.bioassay.tlc;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.Html;
import android.text.InputFilter;
import android.util.Log;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
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

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import uiuc.bioassay.tlc.camera.CameraActivity;
import uiuc.bioassay.tlc.proc.TLCProcActivity;
import uiuc.bioassay.tlc.services.NetworkService;

import static uiuc.bioassay.tlc.TLCApplication.round;


public class TLCResultActivity extends AppCompatActivity {

    private static final String TAG = "TLC RESULT";
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
    private double[] Rf;
    private double[] D;

    private EditText currEditText;
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
            editText.setOnFocusChangeListener(
                    new View.OnFocusChangeListener() {
                        @Override
                        public void onFocusChange(View v, boolean hasFocus) {
                            if (!hasFocus) {
                                InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                                inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                            }
                        }
                    }
            );
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
                        intent.putExtra(TLCApplication.PARENT_FOLDER_EXTRA, rootFolder);
                        intent.putExtra(TLCApplication.CURRENT_IDX, currentIdx);
                        startActivityForResult(intent, GET_DATA_REQUEST);
                    }
                }
        );

        final AlertDialog alertDialog = new AlertDialog.Builder(TLCResultActivity.this).create();
        alertDialog.setTitle("Alert");
        alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                        if (currEditText != null) {
                            currEditText.requestFocus();
                            currEditText = null;
                        }
                    }
                });

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
                        for (int i = 0; i < numConcs; ++i) {
                            Spinner spinner = (Spinner) concTypeRow.getChildAt(i);
                            if (spinner.getSelectedItem().toString().equals("std")) {
                                stdConcs.add(i);
                            } else {
                                unknownConcs.add(i);
                            }
                        }

                        if (unknownConcs.size() == 0) {
                            alertDialog.setMessage("Select at least 1 unknown");
                            alertDialog.show();
                            return;
                        }

                        if (stdConcs.size() < 2) {
                            alertDialog.setMessage("Number of standards is less than 2");
                            alertDialog.show();
                            return;
                        }

                        LinearLayout avgLinearLayout = (LinearLayout) findViewById(R.id.avg_linear_layout);
                        for (int i = 0; i < stdConcs.size(); ++i) {
                            TableRow tr = (TableRow) tableLayout.getChildAt(0);
                            EditText editText = (EditText) tr.getChildAt(stdConcs.get(i));
                            String conc = editText.getText().toString();
                            if (conc.equals("")) {
                                alertDialog.setMessage("Empty standard concentration");
                                alertDialog.show();
                                currEditText = editText;
                                return;
                            }
                            xs.add(Double.parseDouble(conc));
                            LinearLayout linearLayout = (LinearLayout) avgLinearLayout.getChildAt(stdConcs.get(i));
                            TextView textView = (TextView) linearLayout.getChildAt(1);
                            ys.add(Double.parseDouble(textView.getText().toString()));
                        }

                        for (int i = 0; i < unknownConcs.size(); ++i) {
                            LinearLayout linearLayout = (LinearLayout) avgLinearLayout.getChildAt(unknownConcs.get(i));
                            TextView textView = (TextView) linearLayout.getChildAt(1);
                            yUnknown.add(Double.parseDouble(textView.getText().toString()));
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
                            editText.setText(Double.toString(round((yUnknown.get(i) - b) / a, 2)));
                        }
                    }
                }
        );
        Button done = (Button) findViewById(R.id.done);
        final DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which){
                    case DialogInterface.BUTTON_POSITIVE:
                        exportFinalResultToFile();
                        NetworkService.startActionUpload(TLCResultActivity.this, rootFolder + File.separator + TLCApplication.LOG_FILE);
                        finish();
                        break;

                    case DialogInterface.BUTTON_NEGATIVE:
                        //No button clicked
                        break;
                }
            }
        };
        done.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        final AlertDialog.Builder builder = new AlertDialog.Builder(TLCResultActivity.this);
                        builder.setTitle(Html.fromHtml("<font color='#FFF12C'>Warning</font>")).setIcon(R.drawable.alert)
                                .setMessage("Are you sure you are done with the experiment?")
                                .setPositiveButton("Yes", dialogClickListener)
                                .setNegativeButton("No", dialogClickListener);
                        AlertDialog warningDialog = builder.show();
                        // Set title divider color
                        int titleDividerId = getResources().getIdentifier("titleDivider", "id", "android");
                        View titleDivider = warningDialog.findViewById(titleDividerId);
                        if (titleDivider != null)
                            titleDivider.setBackgroundColor(getResources().getColor(android.R.color.holo_blue_dark));
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
                intent.putExtra(TLCApplication.NUM_CONCS, numConcs);
                startActivityForResult(intent, PROCESS_DATA_REQUEST);
            } else {
                Toast.makeText(this, "Fail to get data!", Toast.LENGTH_SHORT).show();
            }
        } else if (requestCode == PROCESS_DATA_REQUEST) {
            if (resultCode == RESULT_OK) {
                double[] res = data.getDoubleArrayExtra(TLCApplication.DATA);
                exportResultToFile(res);
                setData(res);
                for (int i = 0; i < numConcs; ++i) {
                    Rf[i] += res[2 * i];
                    D[i] += res[2 * i + 1];
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
            } else {
                Toast.makeText(this, "Fail to process data!", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void exportResultToFile(double[] res) {
        BufferedWriter out = null;
        try {
            FileWriter fstream = new FileWriter(rootFolder + File.separator + TLCApplication.LOG_FILE, true); //true tells to append data.
            out = new BufferedWriter(fstream);
            out.write("Trial " + currentIdx + ":\r\n");
            for (int i = 0; i < numConcs; ++i) {
                out.write("\tSpot " + i + ":\r\n");
                out.write("\t\tRf: " + res[2*i] + "\r\n");
                out.write("\t\tD: " + res[2*i + 1] + "\r\n");
            }
            out.flush();
        }
        catch (IOException e)
        {
            Log.e(TAG, "Error: " + e.getMessage());
        }
        finally
        {
            if(out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void exportFinalResultToFile() {
        BufferedWriter out = null;
        try {
            FileWriter fstream = new FileWriter(rootFolder + File.separator + TLCApplication.LOG_FILE, true); //true tells to append data.
            out = new BufferedWriter(fstream);
            out.write("Final Result:\r\n");
            TableRow concType = (TableRow) tableLayout.getChildAt(1);
            for (int i = 0; i < numConcs; ++i) {
                Spinner spinner = (Spinner) concType.getChildAt(i);
                out.write("\t" + spinner.getSelectedItem().toString());
                out.write("\t");
            }
            out.write("\r\n");

            TableRow header = (TableRow) tableLayout.getChildAt(0);
            for (int i = 0; i < numConcs; ++i) {
                EditText editText = (EditText) header.getChildAt(i);
                out.write("\t" + editText.getText().toString());
                out.write("\t");
            }
            out.write("\r\n");
            out.flush();
            for (int i = 0; i < numConcs; ++i) {
                out.write("\tRf\tD");
            }
            out.write("\r\n");

            LinearLayout avgLinearLayout = (LinearLayout) findViewById(R.id.avg_linear_layout);
            for (int i = 0; i < numConcs; ++i) {
                out.write("\t" + round(Rf[i]/(currentIdx - 1), 3));
                out.write("\t" + round(D[i]/(currentIdx - 1), 3));
            }
            out.write("\r\n");
        }
        catch (IOException e)
        {
            Log.e(TAG, "Error: " + e.getMessage());
        }
        finally
        {
            if(out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
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
            textView1.setText(Double.toString(round(data[2 * i], 2)));
            linearLayout.addView(textView1);

            TextView textView2 = new TextView(this);
            textView2.setGravity(17);
            textView2.setMinimumWidth(D_COLUMN_WIDTH);
            textView2.setText(Double.toString(round(data[2 * i + 1], 2)));
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
