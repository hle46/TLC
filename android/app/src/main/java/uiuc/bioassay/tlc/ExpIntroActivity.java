package uiuc.bioassay.tlc;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.DatePickerDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.Html;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.EditText;
import android.widget.Toast;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Calendar;

import static uiuc.bioassay.tlc.TLCApplication.cleanFolder;


public class ExpIntroActivity extends AppCompatActivity {
    private static final String TAG = "INTRO";
    private File folder;
    private EditText expName;
    private EditText expDay;
    private EditText userID;
    private EditText userName;
    private EditText phoneNumber;
    private EditText drugINN;
    private EditText lotNumber;
    private EditText expireDay;
    private EditText phoneID;
    private EditText editText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_exp_intro);
        expName = (EditText) findViewById(R.id.exp_name);

        expName.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        if (hasFocus) {
                            InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                            inputMethodManager.showSoftInput(expName, InputMethodManager.SHOW_IMPLICIT);
                        } else {
                            InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                            inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                        }
                    }
                }
        );

        expDay = (EditText) findViewById(R.id.exp_day);
        expDay.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                        if (hasFocus) {
                            v.callOnClick();
                        }
                    }
                }
        );

        expDay.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        Calendar currentDate = Calendar.getInstance();
                        int curYear = currentDate.get(Calendar.YEAR);
                        int curDay = currentDate.get(Calendar.DAY_OF_MONTH);
                        int curMonth = currentDate.get(Calendar.MONTH);


                        DatePickerDialog datePicker;

                        datePicker = new DatePickerDialog(ExpIntroActivity.this, new DatePickerDialog.OnDateSetListener() {

                            public void onDateSet(DatePicker datepicker, int chosenyear, int chosenmonth, int chosenday) {
                                chosenmonth = chosenmonth + 1;
                                expDay.setText(chosenmonth + "/" + chosenday + "/" + chosenyear);
                            }
                        }, curYear, curMonth, curDay);

                        datePicker.setTitle("Select Date");
                        datePicker.show();
                    }
                }
        );

        userID = (EditText) findViewById(R.id.user_ID);
        userID.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        if (hasFocus) {
                            inputMethodManager.toggleSoftInput(0, 0);
                        } else {
                            inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                        }
                    }
                }
        );

        userName = (EditText) findViewById(R.id.user_name);
        userName.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        if (hasFocus) {
                            inputMethodManager.toggleSoftInput(0, 0);
                        } else {
                            inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                        }
                    }
                }
        );

        phoneNumber = (EditText) findViewById(R.id.phone_number);
        phoneNumber.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        if (hasFocus) {
                            inputMethodManager.toggleSoftInput(0, 0);
                        } else {
                            inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                        }
                    }
                }
        );

        drugINN = (EditText) findViewById(R.id.drug_inn);
        drugINN.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        if (hasFocus) {
                            inputMethodManager.toggleSoftInput(0, 0);
                        } else {
                            inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                        }
                    }
                }
        );

        lotNumber = (EditText) findViewById(R.id.lot_number);
        lotNumber.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        if (hasFocus) {
                            inputMethodManager.toggleSoftInput(0, 0);
                        } else {
                            inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                        }
                    }
                }
        );

        expireDay = (EditText) findViewById(R.id.expire_day);
        expireDay.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                        if (hasFocus) {
                            v.callOnClick();
                        }
                    }
                }
        );

        expireDay.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        Calendar currentDate = Calendar.getInstance();
                        int curYear = currentDate.get(Calendar.YEAR);
                        int curDay = currentDate.get(Calendar.DAY_OF_MONTH);
                        int curMonth = currentDate.get(Calendar.MONTH);


                        DatePickerDialog datePicker;

                        datePicker = new DatePickerDialog(ExpIntroActivity.this, new DatePickerDialog.OnDateSetListener() {

                            public void onDateSet(DatePicker datepicker, int chosenyear, int chosenmonth, int chosenday) {
                                chosenmonth = chosenmonth + 1;
                                expireDay.setText(chosenmonth + "/" + chosenday + "/" + chosenyear);
                            }
                        }, curYear, curMonth, curDay);

                        datePicker.setTitle("Select Date");
                        datePicker.show();
                    }
                }
        );

        phoneID = (EditText) findViewById(R.id.phone_id);
        phoneID.setOnFocusChangeListener(
                new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                        if (hasFocus) {
                            inputMethodManager.showSoftInput(phoneID, InputMethodManager.SHOW_IMPLICIT);
                        } else {
                            inputMethodManager.hideSoftInputFromWindow(v.getWindowToken(), 0);
                        }
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
                        startExperiment();
                        break;

                    case DialogInterface.BUTTON_NEGATIVE:
                        //No button clicked
                        break;
                }
            }
        };
        final AlertDialog alertDialog = new AlertDialog.Builder(ExpIntroActivity.this).create();
        alertDialog.setTitle("Alert");
        alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                        editText.requestFocus();
                    }
                });

        expNext.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if (expName.getText().toString().equals("")) {
                            alertDialog.setMessage("Please enter experiment name");
                            alertDialog.show();
                            editText = expName;
                            return;
                        } else if (expDay.getText().toString().equals("")) {
                            alertDialog.setMessage("Please enter experiment day");
                            alertDialog.show();
                            editText = expDay;
                            return;
                        } else if (userID.getText().toString().equals("")) {
                            alertDialog.setMessage("Please enter user ID");
                            alertDialog.show();
                            editText = userID;
                            return;
                        } else if (userName.getText().toString().equals("")) {
                            alertDialog.setMessage("Please enter user name");
                            alertDialog.show();
                            editText = userName;
                            return;
                        } else if (drugINN.getText().toString().equals("")) {
                            alertDialog.setMessage("Please enter drug INN");
                            alertDialog.show();
                            editText = drugINN;
                            return;
                        } else if (lotNumber.getText().toString().equals("")) {
                            alertDialog.setMessage("Please enter lot number");
                            alertDialog.show();
                            editText = lotNumber;
                            return;
                        } else if (expireDay.getText().toString().equals("")) {
                            alertDialog.setMessage("Please enter expire day");
                            alertDialog.show();
                            editText = expireDay;
                            return;
                        }

                        folder = new File(TLCApplication.ROOT_FOLDER, expName.getText().toString());
                        if (folder.exists()) {
                            final AlertDialog.Builder builder = new AlertDialog.Builder(ExpIntroActivity.this);
                            builder.setTitle(Html.fromHtml("<font color='#FFF12C'>Warning</font>")).setIcon(R.drawable.alert)
                                    .setMessage("Experiment name exists. Are you sure you want to overwrite it?")
                                    .setPositiveButton("Yes", dialogClickListener)
                                    .setNegativeButton("No", dialogClickListener);
                            AlertDialog warningDialog = builder.show();
                            // Set title divider color
                            int titleDividerId = getResources().getIdentifier("titleDivider", "id", "android");
                            View titleDivider = warningDialog.findViewById(titleDividerId);
                            if (titleDivider != null)
                                titleDivider.setBackgroundColor(getResources().getColor(android.R.color.holo_blue_dark));
                        } else if (!folder.mkdirs()) {
                            Toast.makeText(ExpIntroActivity.this, "Cannot create folder", Toast.LENGTH_LONG).show();
                            return;
                        } else {
                            startExperiment();
                        }
                    }
                }
        );
    }

    private String getText(EditText editText) {
        String s = editText.getText().toString();
        return (s.equals("") ? "N/A" : s);
    }
    private void startExperiment() {
        // Show confirmation
        DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which){
                    case DialogInterface.BUTTON_POSITIVE:
                        exportInfoToFile();
                        Intent intent = new Intent(ExpIntroActivity.this, PillsActivity.class);
                        intent.putExtra(TLCApplication.FOLDER_EXTRA, folder.getAbsolutePath());
                        startActivity(intent);
                        finish();
                        break;

                    case DialogInterface.BUTTON_NEGATIVE:
                        //No button clicked
                        break;
                }
            }
        };
        AlertDialog.Builder builder = new AlertDialog.Builder(ExpIntroActivity.this);
        builder.setTitle("Confirm").setMessage("Experiment Information: " + "\n  - Name : " + getText(expName) +
                                                                            "\n  - Day : " + getText(expDay) +
                                                                            "\n  - User ID : " + getText(userID) +
                                                                            "\n  - User Name : " + getText(userName) +
                                                                            "\n  - Phone Number : " + getText(phoneNumber) +
                                                                            "\n  - Drug INN : " + getText(drugINN) +
                                                                            "\n  - Lot Number : " + getText(lotNumber) +
                                                                            "\n  - Expire Day : " + getText(expireDay) +
                                                                            "\n  - Phone ID : " + getText(phoneID))
                                    .setPositiveButton("Yes", dialogClickListener)
                                    .setNegativeButton("No", dialogClickListener).show();

    }

    @Override
    public void onBackPressed() {
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



    private void exportInfoToFile() {
        BufferedWriter out = null;
        try
        {
            FileWriter fstream = new FileWriter(folder.getAbsolutePath() + File.separator + TLCApplication.LOG_FILE, true); //true tells to append data.
            out = new BufferedWriter(fstream);
            out.write("Experiment Name: " + getText(expName) + "\r\n");
            out.write("Experiment Day: " + getText(expDay) + "\r\n");
            out.write("User ID: " + getText(userID) + "\r\n");
            out.write("User Name: " + getText(userName) + "\r\n");
            out.write("Phone Number: " + getText(phoneNumber) + "\r\n");
            out.write("Drug Name (INN): " + getText(drugINN) + "\r\n");
            out.write("Lot Number: " + getText(lotNumber) + "\r\n");
            out.write("Expiration Day: " + getText(expireDay) + "\r\n");
            out.write("Phone ID: " + getText(phoneID) + "\r\n");
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
