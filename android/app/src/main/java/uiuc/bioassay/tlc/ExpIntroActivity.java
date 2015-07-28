package uiuc.bioassay.tlc;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.Html;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import uiuc.bioassay.tlc.camera.CameraActivity;

import static uiuc.bioassay.tlc.TLCApplication.cleanFolder;


public class ExpIntroActivity extends AppCompatActivity {
    private static final String TAG = "INTRO";
    private File folder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_exp_intro);
        final EditText expName = (EditText) findViewById(R.id.exp_name);

        expName.setOnFocusChangeListener(
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

        final EditText expDay = (EditText) findViewById(R.id.exp_day);
        expDay.setOnFocusChangeListener(
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

        final EditText userID = (EditText) findViewById(R.id.user_ID);
        userID.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                    }
                }
        );

        final EditText userName = (EditText) findViewById(R.id.user_name);
        userName.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                    }
                }
        );

        final EditText phoneNumber = (EditText) findViewById(R.id.phone_number);
        phoneNumber.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                    }
                }
        );

        final EditText drugINN = (EditText) findViewById(R.id.drug_inn);
        drugINN.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                    }
                }
        );

        final EditText lotNumber = (EditText) findViewById(R.id.lot_number);
        lotNumber.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                    }
                }
        );

        final EditText expireDay = (EditText) findViewById(R.id.expire_day);
        expireDay.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                    }
                }
        );

        final EditText phoneID = (EditText) findViewById(R.id.phone_id);
        phoneID.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                    }
                }
        );

        Button expNext = (Button) findViewById(R.id.exp_next);
        final DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which){
                    case DialogInterface.BUTTON_POSITIVE:
                        cleanFolder(folder.getAbsolutePath());
                        exportToFile(expName.getText().toString(), expDay.getText().toString(), userID.getText().toString(), userName.getText().toString(),
                                phoneNumber.getText().toString(), drugINN.getText().toString(), lotNumber.getText().toString(), expireDay.getText().toString(),
                                phoneID.getText().toString());
                        Intent intent = new Intent(ExpIntroActivity.this, PillsActivity.class);
                        intent.putExtra(TLCApplication.FOLDER_EXTRA, folder.getAbsolutePath());
                        startActivity(intent);
                        break;

                    case DialogInterface.BUTTON_NEGATIVE:
                        //No button clicked
                        break;
                }
            }
        };
        expNext.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        folder = new File(TLCApplication.ROOT_FOLDER, expName.getText().toString());
                        if (folder.exists()) {
                            final AlertDialog.Builder builder = new AlertDialog.Builder(ExpIntroActivity.this);
                            builder.setTitle(Html.fromHtml("<font color='#FFF12C'>Warning</font>")).setIcon(R.drawable.alert)
                                    .setMessage("Experiment name exists. Are you sure you want to overwrite it?")
                                    .setPositiveButton("Yes", dialogClickListener)
                                    .setNegativeButton("No", dialogClickListener);
                            AlertDialog alertDialog = builder.show();
                            // Set title divider color
                            int titleDividerId = getResources().getIdentifier("titleDivider", "id", "android");
                            View titleDivider = alertDialog.findViewById(titleDividerId);
                            if (titleDivider != null)
                                titleDivider.setBackgroundColor(getResources().getColor(android.R.color.holo_blue_dark));
                        } else if (!folder.mkdirs()) {
                            Toast.makeText(ExpIntroActivity.this, "Cannot create folder", Toast.LENGTH_LONG).show();
                            return;
                        } else {
                            exportToFile(expName.getText().toString(), expDay.getText().toString(), userID.getText().toString(), userName.getText().toString(),
                                    phoneNumber.getText().toString(), drugINN.getText().toString(), lotNumber.getText().toString(), expireDay.getText().toString(),
                                    phoneID.getText().toString());
                            Intent intent = new Intent(ExpIntroActivity.this, PillsActivity.class);
                            intent.putExtra(TLCApplication.FOLDER_EXTRA, folder.getAbsolutePath());
                            startActivity(intent);
                        }
                    }
                }
        );
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_exp_intro, menu);
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

    private void exportToFile(String expName, String expDay, String userID, String userName, String phoneNumber, String drugINN, String lotNumber, String expireDay, String phoneID) {
        BufferedWriter out = null;
        try
        {
            FileWriter fstream = new FileWriter(folder.getAbsolutePath() + File.separator + TLCApplication.LOG_FILE, true); //true tells to append data.
            out = new BufferedWriter(fstream);
            out.write("Experiment Name: " + expName + "\n");
            out.write("Experiment Day: " + expDay + "\n");
            out.write("User ID: " + userID + "\n");
            out.write("User Name: " + userName + "\n");
            out.write("Phone Number: " + phoneNumber + "\n");
            out.write("Drug Name (INN): " + drugINN + "\n");
            out.write("Lot Number: " + lotNumber + "\n");
            out.write("Expiration Day: " + expireDay + "\n");
            out.write("Phone ID: " + phoneID + "\n");
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
}
