package com.rtr.window_view_object;

//by default
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

//by user
//for Window
import android.view.Window;

//for WindowManager
import android.view.WindowManager;

//for ActivityInfo where pm = package manager
import android.content.pm.ActivityInfo;

//for Color
import android.graphics.Color;

//for View
import android.view.View;


//text view
import androidx.appcompat.widget.AppCompatTextView;

//for context
import android.content.Context;

//for text alignment
import android.view.Gravity;

//for color
import android.graphics.Color;


public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.activity_main);

        //get rid of title bar
        this.supportRequestWindowFeature(Window.FEATURE_NO_TITLE);

        //get rid of navigation bar
        this.getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_IMMERSIVE);

        //make fullscreen
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,WindowManager.LayoutParams.FLAG_FULLSCREEN);

        //forced landscape orientation
        this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        //set background color
        this.getWindow().getDecorView().setBackgroundColor(Color.BLACK);

        //define your own view
        AppCompatTextView myView = new AppCompatTextView(this);

        myView.setTextColor(Color.rgb(0,255,0));

        //set text size
        myView.setTextSize(60);

        //set text position
        myView.setGravity(Gravity.CENTER);

        myView.setText("Hello World !!!");

        //set this view
        setContentView(myView);
    }

    @Override
    protected void onPause(){
        super.onPause();
    }

    @Override
    protected void onResume(){
        super.onResume();
    }
}
