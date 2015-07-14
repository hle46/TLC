/**
 * University of Illinois
 Open Source License

 Copyright <2015>, <University of Illinois at Urbana-Champaign>. All rights reserved.
 All rights reserved.

 Developed by:

 Smartphone Bioassay Team

 University of Illinois at Urbana-Champaign

 http://sb.illinois.edu

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal with
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimers.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimers in the
 documentation and/or other materials provided with the distribution.

 * Neither the names of the Smartphone Bioassay Team, University of Illinois at
 Urbana-Champaign, nor the names of its contributors may be used to
 endorse or promote products derived from this Software without specific
 prior written permission.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
 SOFTWARE.
 */
package uiuc.bioassay.tlc.camera;

import android.content.Context;
import android.graphics.Rect;
import android.hardware.Camera;
import android.media.MediaActionSound;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.Toast;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

@SuppressWarnings("deprecation")
/** A basic Camera preview class */
public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = "CAMERA PREVIEW";
    private SurfaceHolder mHolder;
    private Camera mCamera;
    private boolean focusOnTouch;
    private FocusSound focusSound;

    private TouchRectView touchRectView = null;

    private boolean focusSuccess = false;

    public boolean isFocusSuccess() {
        return focusSuccess;
    }

    public CameraPreview(Context context, Camera camera) {
        super(context);
        mCamera = camera;

        // Install a SurfaceHolder.Callback so we get notified when the
        // underlying surface is created and destroyed.
        mHolder = getHolder();
        mHolder.addCallback(this);

        // Add focus sound
        focusSound = new FocusSound();
    }

    public void surfaceCreated(SurfaceHolder holder) {
        // The Surface has been created, now tell the tlc where to draw the preview.
        try {
            mCamera.setPreviewDisplay(holder);
            mCamera.startPreview();
        } catch (IOException e) {
            Log.d(TAG, "Error setting tlc preview: " + e.getMessage());
        }
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        // empty. Take care of releasing the Camera preview in your activity.
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        // If your preview can change or rotate, take care of those events here.
        // Make sure to stop the preview before resizing or reformatting it.

        if (mHolder.getSurface() == null){
            // preview surface does not exist
            return;
        }

        // stop preview before making changes
        try {
            mCamera.stopPreview();
        } catch (Exception e){
            // ignore: tried to stop a non-existent preview
        }

        // set preview size and make any resize, rotate or
        // reformatting changes here


        // start preview with new settings
        try {
            mCamera.setPreviewDisplay(mHolder);
            mCamera.startPreview();

        } catch (Exception e){
            Log.d(TAG, "Error starting tlc preview: " + e.getMessage());
        }
    }

    public void setFocusOnTouch(boolean val) {
        focusOnTouch = val;
    }

    /** Works of touch on focus go here */
    public void doTouchFocus(final Rect focusRect, final Rect meteringRect) {
        try {
            // Cancel previous auto focus
            mCamera.cancelAutoFocus();

            // Read, Modify, Write tlc parameters
            // Read parameters
            Camera.Parameters params = mCamera.getParameters();

            // Modify parameters
            // set focus mode in openCamera

            // Check if focus area supported
            if (params.getMaxNumFocusAreas() > 0) {
                // Set focus area
                List<Camera.Area> focusAreas = new ArrayList<>();
                focusAreas.add(new Camera.Area(focusRect, 1000));

                params.setFocusAreas(focusAreas);
            }

            // Check if metering supported
            if (params.getMaxNumMeteringAreas() > 0) {
                // Set metering area
                List<Camera.Area> meteringAreas = new ArrayList<>();
                meteringAreas.add(new Camera.Area(meteringRect, 1000));

                params.setMeteringAreas(meteringAreas);
            }

            // Write parameters
            mCamera.setParameters(params);


            // Perform auto focus
            mCamera.autoFocus(
                    new Camera.AutoFocusCallback() {
                        @Override
                        public void onAutoFocus(boolean success, Camera camera) {
                            if (success) {
                                focusSound.play();
                                focusSuccess = true;
                            } else {
                                focusSuccess = false;
                                Toast.makeText(getContext(), "Fail to focus, try again!", Toast.LENGTH_SHORT).show();
                            }
                        }
                    }
            );
        } catch (Exception e) {
            Toast.makeText(getContext(), "Unable to focus, try again!", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public boolean onTouchEvent(@NonNull MotionEvent event) {
        if (!focusOnTouch) {
            return false;
        }

        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            float x = event.getRawX();
            float y = event.getRawY();
            Rect focusRect = calculateTapArea(x, y, 1f);
            Rect meteringRect = calculateTapArea(event.getRawX(), event.getRawY(), 1.8f);
            doTouchFocus(focusRect, meteringRect);

            final Rect touchRect = new Rect(
                    (int)(x - 25),
                    (int)(y - 25),
                    (int)(x + 25),
                    (int)(y + 25));
            if (touchRectView != null) {
                touchRectView.setRect(touchRect);
                touchRectView.invalidate();

                // Remove the square after some time
                Handler handler = new Handler();
                handler.postDelayed(new Runnable() {

                    @Override
                    public void run() {
                        touchRectView.setRect(new Rect(0, 0, 0, 0));
                        touchRectView.invalidate();
                    }
                }, 1000);
            }

        }
        return false;
    }

    private Camera.Size getResolution() {
        Camera.Parameters params = mCamera.getParameters();
        return params.getPreviewSize();
    }

    /**
     * Convert x, y touch position position -1000:-1000 to 1000:1000.
     */
    private Rect calculateTapArea(float x, float y, float coefficient) {
        float focusAreaSize = 100;
        int areaSize = Float.valueOf(focusAreaSize * coefficient).intValue();

        int centerX = (int) ((x / getWidth()) * 2000 - 1000);
        int centerY = (int) ((y / getHeight()) * 2000 - 1000);

        int left = clamp(centerX - areaSize / 2, -1000, 1000);
        int right = clamp(left + areaSize, -1000, 1000);
        int top = clamp(centerY - areaSize / 2, -1000, 1000);
        int bottom = clamp(top + areaSize, -1000, 1000);

        return new Rect(left, top, right, bottom);
    }

    private int clamp(int x, int min, int max) {
        if (x > max) {
            return max;
        }
        if (x < min) {
            return min;
        }
        return x;
    }

    /** Focus Sound */
    private static class FocusSound {
        private final MediaActionSound media;

        private FocusSound() {
            media = new MediaActionSound();
            media.load(MediaActionSound.FOCUS_COMPLETE);
        }

        public void play() {
            media.play(MediaActionSound.FOCUS_COMPLETE);
        }
    }


    public void setTouchRectView(TouchRectView touchRectView) {
        this.touchRectView = touchRectView;
    }
}
