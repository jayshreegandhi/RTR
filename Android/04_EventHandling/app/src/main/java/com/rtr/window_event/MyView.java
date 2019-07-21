package com.rtr.window_event;

//by user
import androidx.appcompat.widget.AppCompatTextView;
import android.content.Context;
import android.view.Gravity;
import android.graphics.Color;

import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

public class MyView extends AppCompatTextView implements OnGestureListener,OnDoubleTapListener{

    private GestureDetector gestureDetector;

    public MyView(Context drawingContext){
        super(drawingContext);

        //set text color
        setTextColor(Color.rgb(255,128,0));

        //set text size
        setTextSize(60);

        //set text position
        setGravity(Gravity.CENTER);

        setText("Hello World !!!");

        gestureDetector = new GestureDetector(drawingContext,this,null,false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event){

        int eventaction = event.getAction();
        if(!gestureDetector.onTouchEvent(event)){
            super.onTouchEvent(event);
        }

        return(true);
    }

    @Override
    public boolean onDoubleTap(MotionEvent e){
        setText("Double Tap");
        return(true);
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e){
        return(true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e){
        setText("Single Tap");
        return(true);
    }

    @Override
    public boolean onDown(MotionEvent e){
        return(true);
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY){
        return(true);
    }

    @Override
    public void onLongPress(MotionEvent e){
        setText("Long Press");
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2,float distanceX, float distanceY){
        setText("Scroll");
        return(true);
    }

    @Override
    public void onShowPress(MotionEvent e){

    }

    @Override
    public boolean onSingleTapUp(MotionEvent e){
        return(true);
    }
}
