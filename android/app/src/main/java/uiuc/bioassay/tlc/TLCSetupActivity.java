package uiuc.bioassay.tlc;

import android.content.Intent;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.Spinner;


public class TLCSetupActivity extends ActionBarActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_tlcsetup);
        final Spinner numConcsSpinner = (Spinner) findViewById(R.id.num_concs);
        Button nextSetup = (Button) findViewById(R.id.next_setup);
        nextSetup.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        Intent intent = new Intent(TLCSetupActivity.this, TLCResultActivity.class);
                        intent.putExtra(TLCApplication.FOLDER_EXTRA, getIntent().getStringExtra(TLCApplication.FOLDER_EXTRA));
                        intent.putExtra(TLCApplication.NUM_CONCS, Integer.parseInt(numConcsSpinner.getSelectedItem().toString()));
                        startActivity(intent);
                        finish();
                    }
                }
        );

    }

    @Override
    public void onBackPressed() {

    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_tlcsetup, menu);
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
