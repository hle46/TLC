package uiuc.bioassay.tlc.proc;

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.math.BigDecimal;
import java.math.RoundingMode;

import uiuc.bioassay.tlc.R;
import uiuc.bioassay.tlc.TLCApplication;

import static uiuc.bioassay.tlc.TLCApplication.processTLC;

public class TLCProcActivity extends AppCompatActivity {
    private double[] currResult;

    public void setCurrResult(double[] result) {
        currResult = result;
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_tlcproc);
        Button done = (Button) findViewById(R.id.done);
        done.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        finish();
                    }
                }
        );
        TLCProcWorker tlcProcWorker = new TLCProcWorker(this);
        //double[] spots = processTLC(getIntent().getStringExtra(TLCApplication.FOLDER_EXTRA) + "/");
        tlcProcWorker.execute(getIntent().getStringExtra(TLCApplication.FOLDER_EXTRA));
        //Log.d("xxx", getIntent().getStringExtra(TLCApplication.FOLDER_EXTRA));
        //tlcProcWorker.execute("/storage/sdcard0/Android/data/uiuc.bioassay.tlc/n2-4");
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_tlcproc, menu);
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

    @Override
    public void onBackPressed() {
    }
}
