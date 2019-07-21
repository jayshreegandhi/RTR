package com.rtr.window_hello;

//by user
import androidx.appcompat.widget.AppCompatTextView;
import android.content.Context;
import android.view.Gravity;
import android.graphics.Color;

public class MyView extends AppCompatTextView{

    public MyView(Context drawingContext){
        super(drawingContext);

        //set text color
        setTextColor(Color.rgb(0,255,0));

        //set text size
        setTextSize(60);

        //set text position
        setGravity(Gravity.CENTER);

        setText("Hello World !!!");
    }
}
