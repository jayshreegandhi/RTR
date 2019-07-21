package com.rtr.window_println;

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

        System.out.println("RTR: In MyView()");

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

        System.out.println("RTR: In onTouchEvent()");

        int eventaction = event.getAction();
        if(!gestureDetector.onTouchEvent(event)){
            super.onTouchEvent(event);
        }

        return(true);
    }

    @Override
    public boolean onDoubleTap(MotionEvent e){
        System.out.println("RTR: In onDoubleTap()");
        setText("Double Tap");
        return(true);
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e){
        System.out.println("RTR: In onDoubleTapEvent()");
        return(true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e){
        System.out.println("RTR: In onSingleTapConfirmed()");
        setText("Single Tap");
        return(true);
    }

    @Override
    public boolean onDown(MotionEvent e){
        System.out.println("RTR: In onDown()");
        return(true);
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY){
        System.out.println("RTR: In onFling()");
        return(true);
    }

    @Override
    public void onLongPress(MotionEvent e){
        System.out.println("RTR: In onLongPress()");
        setText("Long Press");
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2,float distanceX, float distanceY){
        System.out.println("RTR: In onScroll()");
        System.exit(0);
        return(true);
    }

    @Override
    public void onShowPress(MotionEvent e){
        System.out.println("RTR: In onShowPress()");
    }

    @Override
    public boolean onSingleTapUp(MotionEvent e){
        System.out.println("RTR: In onSingleTapUp()");
        return(true);
    }
}
