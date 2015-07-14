package uiuc.bioassay.tlc.camera;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.View;

/**
 * Created by meowle on 7/1/15.
 */

public class TouchRectView extends View {
    private Rect rect = null;
    private Paint paint;

    public TouchRectView(Context context, AttributeSet attrs) {
        super(context, attrs);
        paint = new Paint();
        paint.setColor(0xeed7d7d7);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(3);
    }

    public void setRect(Rect rect) {
        this.rect = rect;
    }

    public void setRectPaint(Rect rect, Paint paint) {
        this.rect = rect;
        this.paint = paint;
    }

    @Override
    public void onDraw(Canvas canvas) {
        if (rect != null) {
            canvas.drawRect(
                    rect.left, rect.top, rect.right, rect.bottom,
                    paint);
        }
    }

}
