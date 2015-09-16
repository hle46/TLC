package uiuc.bioassay.tlc.proc;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.os.AsyncTask;
import android.view.Gravity;
import android.widget.ImageView;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;

import java.io.File;


import uiuc.bioassay.tlc.R;
import uiuc.bioassay.tlc.TLCApplication;
import uiuc.bioassay.tlc.services.NetworkService;

import static uiuc.bioassay.tlc.TLCApplication.AVG_FILE_NAME;
import static uiuc.bioassay.tlc.TLCApplication.LOG_FILE;
import static uiuc.bioassay.tlc.TLCApplication.SAMPLE_FOLDER;
import static uiuc.bioassay.tlc.TLCApplication.processTLC;
import static uiuc.bioassay.tlc.TLCApplication.round;

/**
 * Created by meowle on 7/8/15.
 */
public class TLCProcWorker extends AsyncTask<String, Void, double[]> {
    private Context mContext;
    private ProgressDialog progressDialog;
    private String folder;

    public TLCProcWorker(Context context) {
        mContext = context;
        progressDialog = new ProgressDialog(context);
    }

    @Override
    protected void onPreExecute() {
        super.onPreExecute();
        progressDialog.setCancelable(false);
        progressDialog.setTitle("Wait");
        progressDialog.setMessage("Processing...");
        progressDialog.show();
    }

    @Override
    protected double[] doInBackground(String... params) {
        folder = params[0];
        return processTLC(folder + File.separator);
    }

    @Override
    protected void onPostExecute(final double[] spots) {
        progressDialog.dismiss();
        for (int i = 0; i < TLCApplication.MAX_PICTURE; ++i) {
            NetworkService.startActionUpload(mContext, folder + File.separator + SAMPLE_FOLDER + AVG_FILE_NAME);
            NetworkService.startActionUpload(mContext, folder + File.separator + LOG_FILE);
        }
        Activity activity = (Activity) mContext;
        TLCProcActivity tlcProcActivity = (TLCProcActivity) activity;
        if (tlcProcActivity == null) {
            return;
        }
        if (spots == null || (spots.length / 2) != tlcProcActivity.getNumConcs()) {
            TextView textView = (TextView) tlcProcActivity.findViewById(R.id.text_result);
            textView.setText("Error, unable to process data!");
            tlcProcActivity.setCurrResult(null);
            return;
        }
        ImageView imageView = (ImageView) tlcProcActivity.findViewById(R.id.imageView);
        imageView.setImageBitmap(decodeIMG(folder + File.separator + TLCApplication.SAMPLE_FOLDER + AVG_FILE_NAME));

        TableLayout tableLayout = (TableLayout) tlcProcActivity.findViewById(R.id.results);
        TableRow tr1 = new TableRow(tlcProcActivity);
        for (int j = 0; j < spots.length / 2; ++j) {
            TextView textView = new TextView(tlcProcActivity);
            textView.setTextColor(tlcProcActivity.getResources().getColor(R.color.black));
            textView.setGravity(Gravity.CENTER_HORIZONTAL);
            textView.setWidth(90);
            textView.setTextSize(18);
            textView.setText("Rf: " + round(spots[2 * j], 2));
            tr1.addView(textView);
        }
        tableLayout.addView(tr1);

        TableRow tr2 = new TableRow(tlcProcActivity);
        for (int j = 0; j < spots.length / 2; ++j) {
            TextView textView = new TextView(tlcProcActivity);
            textView.setTextColor(tlcProcActivity.getResources().getColor(R.color.black));
            textView.setGravity(Gravity.CENTER_HORIZONTAL);
            textView.setWidth(90);
            textView.setTextSize(18);
            textView.setText("D: " + round(spots[2 * j + 1], 2));
            tr2.addView(textView);
        }
        tableLayout.addView(tr2);
        tlcProcActivity.setCurrResult(spots);
    }

    private static Bitmap decodeIMG(String img) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inSampleSize = 4;
        Bitmap bitmap = BitmapFactory.decodeFile(img, options);
        Matrix matrix = new Matrix();
        matrix.postRotate(90);
        return Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
    }
}
