package com.none.non.openhd;

import android.content.res.ColorStateList;
import android.graphics.Color;
import android.os.Bundle;
//import android.support.design.widget.FloatingActionButton;
//import android.support.design.widget.Snackbar;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;


import android.os.Handler;
import android.widget.EditText;
import android.widget.TextView;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketAddress;

import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Toast;


import org.w3c.dom.Text;


public class MainActivity extends AppCompatActivity implements View.OnClickListener,
        AdapterView.OnItemSelectedListener {

        final Handler handler = new Handler();


    public String IPstr = null;
    private TextView TextStatusShow;
    private WFBCDataModel DataModel;

    private ArrayAdapter<CharSequence> adapter;
    public Spinner mySpinner;

    public Spinner SpinnerImperial;
    private ArrayAdapter<CharSequence> adapterImperial;

    public Spinner SpinnerCopter;
    private ArrayAdapter<CharSequence> adapterCopter;


    public Spinner SpinnerFC_RC_BAUDRATE;
    private ArrayAdapter<CharSequence> adapterFC_RC_BAUDRATE;

    public Spinner SpinnerFC_TELEMETRY_BAUDRATE;
    private ArrayAdapter<CharSequence> adapterFC_TELEMETRY_BAUDRATE;

    public Spinner SpinnerFC_MSP_BAUDRATE;
    private ArrayAdapter<CharSequence> adapterFC_MSP_BAUDRATE;

    public Spinner SpinnerEncryptionOrRange;
    private ArrayAdapter<CharSequence> adapterEncryptionOrRange;






    private ViewPager mViewPager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
       // Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
       // setSupportActionBar(toolbar);

        DataModel = new WFBCDataModel();


        TextStatusShow = (TextView) findViewById(R.id.TextStatusShow);

        startServerSocket();

        //Spinner Freq
        mySpinner = (Spinner) findViewById(R.id.spinnerFreq);

        adapter = ArrayAdapter.createFromResource(
                this, R.array.FreqArray, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mySpinner.setAdapter(adapter);
        mySpinner.setOnItemSelectedListener(this);

        //Imperial
        SpinnerImperial = (Spinner) findViewById(R.id.spinnerImperial);
        adapterImperial = ArrayAdapter.createFromResource(
                this, R.array.ImperialArray, android.R.layout.simple_spinner_item);
        adapterImperial.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        SpinnerImperial.setAdapter(adapterImperial);
        SpinnerImperial.setOnItemSelectedListener(new OnItemSelectedListener() {

            @Override
            public void onItemSelected(AdapterView<?> parent, View view,
                                       int position, long id) {
                String sSelected=parent.getItemAtPosition(position).toString();
                //Toast.makeText(getApplicationContext(),sSelected,Toast.LENGTH_SHORT).show();

                if(sSelected.equals("value not loaded") == false)
                    DataModel.AddData("Imperial", sSelected,1);
                if(DataModel.ImperialIsChanged >= 1)
                {
                    SpinnerImperial.setBackgroundColor(Color.parseColor("#ee8033"));
                }

            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });

        //Copter
        SpinnerCopter = (Spinner) findViewById(R.id.spinnerCopter);
        adapterCopter = ArrayAdapter.createFromResource(
                this, R.array.CopterArray, android.R.layout.simple_spinner_item);
        adapterCopter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        SpinnerCopter.setAdapter(adapterCopter);
        SpinnerCopter.setOnItemSelectedListener(new OnItemSelectedListener() {

                @Override
                public void onItemSelected(AdapterView<?> parent, View view,
                int position, long id) {
                    String sSelected=parent.getItemAtPosition(position).toString();
                    //Toast.makeText(getApplicationContext(),sSelected,Toast.LENGTH_SHORT).show();
                    if(sSelected.equals("value not loaded") == false)
                        DataModel.AddData("Copter", sSelected,1);
                    if(DataModel.CopterIsChanged >= 1)
                    {
                        SpinnerCopter.setBackgroundColor(Color.parseColor("#ee8033"));
                    }

                }

                @Override
                public void onNothingSelected(AdapterView<?> adapterView) {

                }
            });

        //spinnerFC_RC_BAUDRATE
        SpinnerFC_RC_BAUDRATE = (Spinner) findViewById(R.id.spinnerFC_RC_BAUDRATE);
        adapterFC_RC_BAUDRATE = ArrayAdapter.createFromResource(
                this, R.array.RateArray, android.R.layout.simple_spinner_item);
        adapterFC_RC_BAUDRATE.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        SpinnerFC_RC_BAUDRATE.setAdapter(adapterFC_RC_BAUDRATE);
        SpinnerFC_RC_BAUDRATE.setOnItemSelectedListener(new OnItemSelectedListener() {

            @Override
            public void onItemSelected(AdapterView<?> parent, View view,
                                       int position, long id) {
                String sSelected=parent.getItemAtPosition(position).toString();
                //Toast.makeText(getApplicationContext(),sSelected,Toast.LENGTH_SHORT).show();
                if(sSelected.equals("value not loaded") == false)
                    DataModel.AddData("FC_RC_BAUDRATE", sSelected,1);
                if(DataModel.FC_RC_BAUDRATEIsChanged >= 1)
                {
                    SpinnerFC_RC_BAUDRATE.setBackgroundColor(Color.parseColor("#ee8033"));
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });


        //SpinnerFC_TELEMETRY_BAUDRATE
        SpinnerFC_TELEMETRY_BAUDRATE = (Spinner) findViewById(R.id.spinnerFC_TELEMETRY_BAUDRATE);
        adapterFC_TELEMETRY_BAUDRATE = ArrayAdapter.createFromResource(
                this, R.array.RateArray, android.R.layout.simple_spinner_item);
        adapterFC_TELEMETRY_BAUDRATE.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        SpinnerFC_TELEMETRY_BAUDRATE.setAdapter(adapterFC_TELEMETRY_BAUDRATE);
        SpinnerFC_TELEMETRY_BAUDRATE.setOnItemSelectedListener(new OnItemSelectedListener() {

            @Override
            public void onItemSelected(AdapterView<?> parent, View view,
                                       int position, long id) {
                String sSelected=parent.getItemAtPosition(position).toString();
                //Toast.makeText(getApplicationContext(),sSelected,Toast.LENGTH_SHORT).show();
                if(sSelected.equals("value not loaded") == false)
                    DataModel.AddData("FC_TELEMETRY_BAUDRATE", sSelected,1);
                if(DataModel.FC_TELEMETRY_BAUDRATEIsChanged >= 1)
                {
                    SpinnerFC_TELEMETRY_BAUDRATE.setBackgroundColor(Color.parseColor("#ee8033"));
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });


        //spinnerFC_MSP_BAUDRATE
        SpinnerFC_MSP_BAUDRATE = (Spinner) findViewById(R.id.spinnerFC_MSP_BAUDRATE);
        adapterFC_MSP_BAUDRATE = ArrayAdapter.createFromResource(
                this, R.array.RateArray, android.R.layout.simple_spinner_item);
        adapterFC_MSP_BAUDRATE.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        SpinnerFC_MSP_BAUDRATE.setAdapter(adapterFC_MSP_BAUDRATE);
        SpinnerFC_MSP_BAUDRATE.setOnItemSelectedListener(new OnItemSelectedListener() {

            @Override
            public void onItemSelected(AdapterView<?> parent, View view,
                                       int position, long id) {
                String sSelected=parent.getItemAtPosition(position).toString();
                //Toast.makeText(getApplicationContext(),sSelected,Toast.LENGTH_SHORT).show();
                if(sSelected.equals("value not loaded") == false)
                    DataModel.AddData("FC_MSP_BAUDRATE", sSelected,1);
                if(DataModel.FC_MSP_BAUDRATEIsChanged >= 1)
                {
                    SpinnerFC_MSP_BAUDRATE.setBackgroundColor(Color.parseColor("#ee8033"));
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });


        //SpinnerEncryptionOrRange
        SpinnerEncryptionOrRange = (Spinner) findViewById(R.id.spinnerEncryptionOrRange);
        adapterEncryptionOrRange = ArrayAdapter.createFromResource(
                this, R.array.EncryptionOrRangeArray, android.R.layout.simple_spinner_item);
        adapterEncryptionOrRange.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        SpinnerEncryptionOrRange.setAdapter(adapterEncryptionOrRange);
        SpinnerEncryptionOrRange.setOnItemSelectedListener(new OnItemSelectedListener() {

            @Override
            public void onItemSelected(AdapterView<?> parent, View view,
                                       int position, long id) {
                String sSelected=parent.getItemAtPosition(position).toString();
                //Toast.makeText(getApplicationContext(),sSelected,Toast.LENGTH_SHORT).show();
                if(sSelected.equals("value not loaded") == false)
                    DataModel.AddData("EncryptionOrRange", sSelected,1);
                if(DataModel.EncryptionOrRangeIsChanged >= 1)
                {
                    SpinnerEncryptionOrRange.setBackgroundColor(Color.parseColor("#ee8033"));
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });




        AddEditBoxListeners();

        //

    }

    public void AddEditBoxListeners()
    {

        //FC_RC_SERIALPORT
        final EditText DefaultAudioOut = (EditText) findViewById(R.id.DefaultAudioOut);
        DefaultAudioOut.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("DefaultAudioOut", mesg,1);
                if(DataModel.DefaultAudioOutIsChanged >= 1)
                {
                    DefaultAudioOut.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.DefaultAudioOutb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //FC_RC_SERIALPORT
        final EditText IsAudioTransferEnabled = (EditText) findViewById(R.id.IsAudioTransferEnabled);
        IsAudioTransferEnabled.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("IsAudioTransferEnabled", mesg,1);
                if(DataModel.IsAudioTransferEnabledIsChanged >= 1)
                {
                    IsAudioTransferEnabled.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.IsAudioTransferEnabledb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //FC_RC_SERIALPORT
        final EditText txpowerA = (EditText) findViewById(R.id.txpowerA);
        txpowerA.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("txpowerA", mesg,1);
                if(DataModel.txpowerAIsChanged >= 1)
                {
                    txpowerA.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.txpowerAb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //FC_RC_SERIALPORT
        final EditText txpowerR = (EditText) findViewById(R.id.txpowerR);
        txpowerR.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("txpowerR", mesg,1);
                if(DataModel.txpowerRIsChanged >= 1)
                {
                    txpowerR.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.txpowerRb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //FC_RC_SERIALPORT
        final EditText UPDATE_NTH_TIME_l = (EditText) findViewById(R.id.UPDATE_NTH_TIME);
        UPDATE_NTH_TIME_l.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("UPDATE_NTH_TIME", mesg,1);
                if(DataModel.UPDATE_NTH_TIMEIsChanged >= 1)
                {
                    UPDATE_NTH_TIME_l.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.UPDATE_NTH_TIMEb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });


        //FC_RC_SERIALPORT
        final EditText RemoteSettingsEnabled = (EditText) findViewById(R.id.RemoteSettingsEnabled);
        RemoteSettingsEnabled.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("RemoteSettingsEnabled", mesg,1);
                if(DataModel.RemoteSettingsEnabledIsChanged >= 1)
                {
                    RemoteSettingsEnabled.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.RemoteSettingsEnabledb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });



        //FC_RC_SERIALPORT
        final EditText FC_TELEMETRY_SERIALPORT = (EditText) findViewById(R.id.FC_TELEMETRY_SERIALPORT);
        FC_TELEMETRY_SERIALPORT.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("FC_TELEMETRY_SERIALPORT", mesg,1);
                if(DataModel.FC_TELEMETRY_SERIALPORTIsChanged >= 1)
                {
                    FC_TELEMETRY_SERIALPORT.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.FC_TELEMETRY_SERIALPORTb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });


        //FC_RC_SERIALPORT
        final EditText FC_RC_SERIALPORT = (EditText) findViewById(R.id.FC_RC_SERIALPORT);
        FC_RC_SERIALPORT.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("FC_RC_SERIALPORT", mesg,1);
                if(DataModel.FC_RC_SERIALPORTIsChanged >= 1)
                {
                    FC_RC_SERIALPORT.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.FC_RC_SERIALPORTb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });


        //FC_RC_SERIALPORT
        final EditText FC_MSP_SERIALPORT = (EditText) findViewById(R.id.FC_MSP_SERIALPORT);
        FC_MSP_SERIALPORT.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("FC_MSP_SERIALPORT", mesg,1);
                if(DataModel.FC_MSP_SERIALPORTIsChanged >= 1)
                {
                    FC_MSP_SERIALPORT.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.FC_MSP_SERIALPORTb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });



        //DATARATE
        final EditText DATARATE = (EditText) findViewById(R.id.DATARATE);
        DATARATE.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {


                String mesg = s.toString();
                DataModel.AddData("DATARATE", mesg,1);
                Log.v("DATARATE", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.DATARATEIsChanged)  );
                if(DataModel.DATARATEIsChanged >= 1)
                    {
                        DATARATE.setTextColor(Color.parseColor("#ee8033") );
                    }
                }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.DATARATEb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });
        ////EditVideoBlock
        final EditText VIDEO_BLOCKS = (EditText) findViewById(R.id.VIDEO_BLOCKS);
        VIDEO_BLOCKS.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("VIDEO_BLOCKS", mesg,1);
                Log.v("VIDEO_BLOCKS", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.VIDEO_BLOCKSIsChanged)  );
                if(DataModel.VIDEO_BLOCKSIsChanged >= 1)
                {
                    VIDEO_BLOCKS.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.VIDEO_BLOCKSb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });
//_______________________________________
        //VIDEO_FECS
        final EditText VIDEO_FECS = (EditText) findViewById(R.id.VIDEO_FECS);
        VIDEO_FECS.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("VIDEO_FECS", mesg,1);
                Log.v("VIDEO_FECS", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.VIDEO_FECSIsChanged)  );
                if(DataModel.VIDEO_FECSIsChanged >= 1)
                {
                    VIDEO_FECS.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.VIDEO_FECSb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });
        //KEYFRAMERATE
        final EditText KEYFRAMERATE = (EditText) findViewById(R.id.KEYFRAMERATE);
        KEYFRAMERATE.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("KEYFRAMERATE", mesg,1);
                Log.v("KEYFRAMERATE", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.KEYFRAMERATEIsChanged)  );
                if(DataModel.KEYFRAMERATEIsChanged >= 1)
                {
                    KEYFRAMERATE.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.KEYFRAMERATEb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });
        //EXTRAPARAMS
        final EditText EXTRAPARAMS = (EditText) findViewById(R.id.EXTRAPARAMS);
        EXTRAPARAMS.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("EXTRAPARAMS", mesg,1);
                Log.v("EXTRAPARAMS", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.EXTRAPARAMSIsChanged)  );
                if(DataModel.EXTRAPARAMSIsChanged >= 1)
                {
                    EXTRAPARAMS.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.EXTRAPARAMSb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });
        //HEIGHT
        final EditText HEIGHT = (EditText) findViewById(R.id.HEIGHT);
        HEIGHT.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("HEIGHT", mesg,1);
                Log.v("HEIGHT", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.HEIGHTIsChanged)  );
                if(DataModel.HEIGHTIsChanged >= 1)
                {
                    HEIGHT.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.HEIGHTb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });
        //HEIGHT
        final EditText WIDTH = (EditText) findViewById(R.id.WIDTH);
        WIDTH.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("WIDTH", mesg,1);
                Log.v("WIDTH", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.WIDTHIsChanged)  );
                if(DataModel.WIDTHIsChanged >= 1)
                {
                    WIDTH.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.WIDTHb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });
        //VIDEO_BLOCKLENGTH
        final EditText VIDEO_BLOCKLENGTH = (EditText) findViewById(R.id.VIDEO_BLOCKLENGTH);
        VIDEO_BLOCKLENGTH.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("VIDEO_BLOCKLENGTH", mesg,1);
                Log.v("VIDEO_BLOCKLENGTH", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.VIDEO_BLOCKLENGTHIsChanged)  );
                if(DataModel.VIDEO_BLOCKLENGTHIsChanged >= 1)
                {
                    VIDEO_BLOCKLENGTH.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.VIDEO_BLOCKLENGTHb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });
        //VIDEO_BITRATE
        final EditText VIDEO_BITRATE = (EditText) findViewById(R.id.VIDEO_BITRATE);
        VIDEO_BITRATE.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("VIDEO_BITRATE", mesg,1);
                Log.v("VIDEO_BITRATE", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.VIDEO_BITRATEIsChanged)  );
                if(DataModel.VIDEO_BITRATEIsChanged >= 1)
                {
                    VIDEO_BITRATE.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.VIDEO_BITRATEb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });
        //BITRATE_PERCENT
        final EditText BITRATE_PERCENT = (EditText) findViewById(R.id.BITRATE_PERCENT);
        BITRATE_PERCENT.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("BITRATE_PERCENT", mesg,1);
                Log.v("BITRATE_PERCENT", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.BITRATE_PERCENTIsChanged)  );
                if(DataModel.BITRATE_PERCENTIsChanged >= 1)
                {
                    BITRATE_PERCENT.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.BITRATE_PERCENTb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //FPS
        final EditText FPS = (EditText) findViewById(R.id.FPS);
        FPS.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("FPS", mesg,1);
                Log.v("FPS", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.FPSIsChanged)  );
                if(DataModel.FPSIsChanged >= 1)
                {
                    FPS.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.FPSb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //FREQSCAN
        final EditText FREQSCAN = (EditText) findViewById(R.id.FREQSCAN);
        FREQSCAN.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("FREQSCAN", mesg,1);
                Log.v("FREQSCAN", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.FREQSCANIsChanged)  );
                if(DataModel.FREQSCANIsChanged >= 1)
                {
                    FREQSCAN.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.FREQSCANb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });


        //TXMODE
        final EditText TXMODE = (EditText) findViewById(R.id.TXMODE);
        TXMODE.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("TXMODE", mesg,1);
                Log.v("TXMODE", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.TXMODEIsChanged)  );
                if(DataModel.TXMODEIsChanged >= 1)
                {
                    TXMODE.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.TXMODEb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //TELEMETRY_TRANSMISSION
        final EditText TELEMETRY_TRANSMISSION = (EditText) findViewById(R.id.TELEMETRY_TRANSMISSION);
        TELEMETRY_TRANSMISSION.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("TELEMETRY_TRANSMISSION", mesg,1);
                Log.v("TELEMETRY_TRANSMISSION", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.TELEMETRY_TRANSMISSIONIsChanged)  );
                if(DataModel.TELEMETRY_TRANSMISSIONIsChanged >= 1)
                {
                    TELEMETRY_TRANSMISSION.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.TELEMETRY_TRANSMISSIONb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //TELEMETRY_UPLINK
        final EditText TELEMETRY_UPLINK = (EditText) findViewById(R.id.TELEMETRY_UPLINK);
        TELEMETRY_UPLINK.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("TELEMETRY_UPLINK", mesg,1);
                Log.v("TELEMETRY_UPLINK", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.TELEMETRY_UPLINKIsChanged)  );
                if(DataModel.TELEMETRY_UPLINKIsChanged >= 1)
                {
                    TELEMETRY_UPLINK.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.TELEMETRY_UPLINKb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //RC
        final EditText RC = (EditText) findViewById(R.id.RC);
        RC.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("RC", mesg,1);
                Log.v("RC", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.RCIsChanged)  );
                if(DataModel.RCIsChanged >= 1)
                {
                    RC.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.RCb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //CTS_PROTECTION
        final EditText CTS_PROTECTION = (EditText) findViewById(R.id.CTS_PROTECTION);
        CTS_PROTECTION.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("CTS_PROTECTION", mesg,1);
                Log.v("CTS_PROTECTION", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.CTS_PROTECTIONIsChanged)  );
                if(DataModel.CTS_PROTECTIONIsChanged >= 1)
                {
                    CTS_PROTECTION.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.CTS_PROTECTIONb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //WIFI_HOTSPOT
        final EditText WIFI_HOTSPOT = (EditText) findViewById(R.id.WIFI_HOTSPOT);
        WIFI_HOTSPOT.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("WIFI_HOTSPOT", mesg,1);
                Log.v("WIFI_HOTSPOT", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.WIFI_HOTSPOTIsChanged)  );
                if(DataModel.WIFI_HOTSPOTIsChanged >= 1)
                {
                    WIFI_HOTSPOT.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.WIFI_HOTSPOTb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //WIFI_HOTSPOT_NIC
        final EditText WIFI_HOTSPOT_NIC = (EditText) findViewById(R.id.WIFI_HOTSPOT_NIC);
        WIFI_HOTSPOT_NIC.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("WIFI_HOTSPOT_NIC", mesg,1);
                Log.v("WIFI_HOTSPOT_NIC", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.WIFI_HOTSPOT_NICIsChanged)  );
                if(DataModel.WIFI_HOTSPOT_NICIsChanged >= 1)
                {
                    WIFI_HOTSPOT_NIC.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.WIFI_HOTSPOT_NICb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //ETHERNET_HOTSPOT
        final EditText ETHERNET_HOTSPOT = (EditText) findViewById(R.id.ETHERNET_HOTSPOT);
        ETHERNET_HOTSPOT.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("ETHERNET_HOTSPOT", mesg,1);
                Log.v("ETHERNET_HOTSPOT", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.ETHERNET_HOTSPOTIsChanged)  );
                if(DataModel.ETHERNET_HOTSPOTIsChanged >= 1)
                {
                    ETHERNET_HOTSPOT.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.ETHERNET_HOTSPOTb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //ENABLE_SCREENSHOTS
        final EditText ENABLE_SCREENSHOTS = (EditText) findViewById(R.id.ENABLE_SCREENSHOTS);
        ENABLE_SCREENSHOTS.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("ENABLE_SCREENSHOTS", mesg,1);
                Log.v("ENABLE_SCREENSHOTS", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.ENABLE_SCREENSHOTSIsChanged)  );
                if(DataModel.ENABLE_SCREENSHOTSIsChanged >= 1)
                {
                    ENABLE_SCREENSHOTS.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.ENABLE_SCREENSHOTSb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //FORWARD_STREAM
        final EditText FORWARD_STREAM = (EditText) findViewById(R.id.FORWARD_STREAM);
        FORWARD_STREAM.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("FORWARD_STREAM", mesg,1);
                Log.v("FORWARD_STREAM", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.FORWARD_STREAMIsChanged)  );
                if(DataModel.FORWARD_STREAMIsChanged >= 1)
                {
                    FORWARD_STREAM.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.FORWARD_STREAMb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });


        //VIDEO_UDP_PORT
        final EditText VIDEO_UDP_PORT = (EditText) findViewById(R.id.VIDEO_UDP_PORT);
        VIDEO_UDP_PORT.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("VIDEO_UDP_PORT", mesg,1);
                Log.v("VIDEO_UDP_PORT", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.VIDEO_UDP_PORTIsChanged)  );
                if(DataModel.VIDEO_UDP_PORTIsChanged >= 1)
                {
                    VIDEO_UDP_PORT.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.VIDEO_UDP_PORTb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Camera Control
        //IsCamera1Enabled
        final EditText IsCamera1Enabled = (EditText) findViewById(R.id.IsCamera1Enabled);
        IsCamera1Enabled.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("IsCamera1Enabled", mesg,1);
                Log.v("IsCamera1Enabled", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.IsCamera1EnabledIsChanged)  );
                if(DataModel.IsCamera1EnabledIsChanged >= 1)
                {
                    IsCamera1Enabled.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.IsCamera1Enabledb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //IsCamera2Enabled
        final EditText IsCamera2Enabled = (EditText) findViewById(R.id.IsCamera2Enabled);
        IsCamera2Enabled.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("IsCamera2Enabled", mesg,1);
                Log.v("IsCamera2Enabled", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.IsCamera2EnabledIsChanged)  );
                if(DataModel.IsCamera2EnabledIsChanged >= 1)
                {
                    IsCamera2Enabled.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.IsCamera2Enabledb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //IsCamera3Enabled
        final EditText IsCamera3Enabled = (EditText) findViewById(R.id.IsCamera3Enabled);
        IsCamera3Enabled.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("IsCamera3Enabled", mesg,1);
                Log.v("IsCamera3Enabled", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.IsCamera3EnabledIsChanged)  );
                if(DataModel.IsCamera3EnabledIsChanged >= 1)
                {
                    IsCamera3Enabled.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.IsCamera3Enabledb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //IsCamera4Enabled
        final EditText IsCamera4Enabled = (EditText) findViewById(R.id.IsCamera4Enabled);
        IsCamera4Enabled.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("IsCamera4Enabled", mesg,1);
                Log.v("IsCamera4Enabled", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.IsCamera4EnabledIsChanged)  );
                if(DataModel.IsCamera4EnabledIsChanged >= 1)
                {
                    IsCamera4Enabled.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.IsCamera4Enabledb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //DefaultCameraId
        final EditText DefaultCameraId = (EditText) findViewById(R.id.DefaultCameraId);
        DefaultCameraId.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("DefaultCameraId", mesg,1);
                Log.v("DefaultCameraId", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.DefaultCameraIdIsChanged)  );
                if(DataModel.DefaultCameraIdIsChanged >= 1)
                {
                    DefaultCameraId.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.DefaultCameraIdb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //ChannelToListen
        final EditText ChannelToListen = (EditText) findViewById(R.id.ChannelToListen);
        ChannelToListen.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("ChannelToListen", mesg,1);
                Log.v("ChannelToListen", mesg);
                Log.v("CounterState: ", Integer.toString(DataModel.ChannelToListenIsChanged)  );
                if(DataModel.ChannelToListenIsChanged >= 1)
                {
                    ChannelToListen.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.ChannelToListenb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Camera1ValueMin
        final EditText Camera1ValueMin = (EditText) findViewById(R.id.Camera1ValueMin);
        Camera1ValueMin.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Camera1ValueMin", mesg,1);
                Log.v("Camera1ValueMin", mesg);
                if(DataModel.Camera1ValueMinIsChanged >= 1)
                {
                    Camera1ValueMin.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Camera1ValueMinb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Camera1ValueMax
        final EditText Camera1ValueMax = (EditText) findViewById(R.id.Camera1ValueMax);
        Camera1ValueMax.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Camera1ValueMax", mesg,1);
                Log.v("Camera1ValueMax", mesg);
                if(DataModel.Camera1ValueMaxIsChanged >= 1)
                {
                    Camera1ValueMax.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Camera1ValueMaxb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Camera2ValueMin
        final EditText Camera2ValueMin = (EditText) findViewById(R.id.Camera2ValueMin);
        Camera2ValueMin.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Camera2ValueMin", mesg,1);
                Log.v("Camera2ValueMin", mesg);
                if(DataModel.Camera2ValueMinIsChanged >= 1)
                {
                    Camera2ValueMin.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Camera2ValueMinb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Camera2ValueMax
        final EditText Camera2ValueMax = (EditText) findViewById(R.id.Camera2ValueMax);
        Camera2ValueMax.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Camera2ValueMax", mesg,1);
                Log.v("Camera2ValueMax", mesg);
                if(DataModel.Camera2ValueMaxIsChanged >= 1)
                {
                    Camera2ValueMax.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Camera2ValueMaxb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Camera3ValueMin
        final EditText Camera3ValueMin = (EditText) findViewById(R.id.Camera3ValueMin);
        Camera3ValueMin.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Camera3ValueMin", mesg,1);
                Log.v("Camera3ValueMin", mesg);
                if(DataModel.Camera3ValueMinIsChanged >= 1)
                {
                    Camera3ValueMin.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Camera3ValueMinb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Camera3ValueMax
        final EditText Camera3ValueMax = (EditText) findViewById(R.id.Camera3ValueMax);
        Camera3ValueMax.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Camera3ValueMax", mesg,1);
                Log.v("Camera3ValueMax", mesg);
                if(DataModel.Camera3ValueMaxIsChanged >= 1)
                {
                    Camera3ValueMax.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Camera3ValueMaxb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Camera4ValueMin
        final EditText Camera4ValueMin = (EditText) findViewById(R.id.Camera4ValueMin);
        Camera4ValueMin.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Camera4ValueMin", mesg,1);
                Log.v("Camera4ValueMin", mesg);
                if(DataModel.Camera4ValueMinIsChanged >= 1)
                {
                    Camera4ValueMin.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Camera4ValueMinb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Camera4ValueMax
        final EditText Camera4ValueMax = (EditText) findViewById(R.id.Camera4ValueMax);
        Camera4ValueMax.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Camera4ValueMax", mesg,1);
                Log.v("Camera4ValueMax", mesg);
                if(DataModel.Camera4ValueMaxIsChanged >= 1)
                {
                    Camera4ValueMax.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Camera4ValueMaxb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Cells
        final EditText CELLS = (EditText) findViewById(R.id.CELLS);
        CELLS.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("CELLS", mesg,1);
                Log.v("CELLS", mesg);
                if(DataModel.CELLSIsChanged >= 1)
                {
                    CELLS.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.CELLSb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //IsBandSwicherEnabled
        final EditText IsBandSwicherEnabled = (EditText) findViewById(R.id.IsBandSwicherEnabled);
        IsBandSwicherEnabled.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("IsBandSwicherEnabled", mesg,1);
                Log.v("IsBandSwicherEnabled", mesg);
                if(DataModel.IsBandSwicherEnabledIsChanged >= 1)
                {
                    IsBandSwicherEnabled.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.IsBandSwicherEnabledb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Bandwidth
        final EditText Bandwidth = (EditText) findViewById(R.id.Bandwidth);
        Bandwidth.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Bandwidth", mesg,1);
                Log.v("Bandwidth", mesg);
                if(DataModel.BandwidthIsChanged >= 1)
                {
                    Bandwidth.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Bandwidthb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //UplinkSpeed
        final EditText UplinkSpeed = (EditText) findViewById(R.id.UplinkSpeed);
        UplinkSpeed.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("UplinkSpeed", mesg,1);
                Log.v("UplinkSpeed", mesg);
                if(DataModel.UplinkSpeedIsChanged >= 1)
                {
                    UplinkSpeed.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.UplinkSpeedb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //ChannelToListen2
        final EditText ChannelToListen2 = (EditText) findViewById(R.id.ChannelToListen2);
        ChannelToListen2.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("ChannelToListen2", mesg,1);
                Log.v("ChannelToListen2", mesg);
                if(DataModel.ChannelToListen2IsChanged >= 1)
                {
                    ChannelToListen2.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.ChannelToListen2b = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //PrimaryCardMAC
        final EditText PrimaryCardMAC = (EditText) findViewById(R.id.PrimaryCardMAC);
        PrimaryCardMAC.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("PrimaryCardMAC", mesg,1);
                Log.v("PrimaryCardMAC", mesg);
                if(DataModel.PrimaryCardMACIsChanged >= 1)
                {
                    PrimaryCardMAC.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.PrimaryCardMACb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //SlaveCardMAC
        final EditText SlaveCardMAC = (EditText) findViewById(R.id.SlaveCardMAC);
        SlaveCardMAC.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("SlaveCardMAC", mesg,1);
                Log.v("SlaveCardMAC", mesg);
                if(DataModel.SlaveCardMACIsChanged >= 1)
                {
                    SlaveCardMAC.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.SlaveCardMACb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Band5Below
        final EditText Band5Below = (EditText) findViewById(R.id.Band5Below);
        Band5Below.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Band5Below", mesg,1);
                Log.v("Band5Below", mesg);
                if(DataModel.Band5BelowIsChanged >= 1)
                {
                    Band5Below.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Band5Belowb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Band10ValueMin
        final EditText Band10ValueMin = (EditText) findViewById(R.id.Band10ValueMin);
        Band10ValueMin.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Band10ValueMin", mesg,1);
                Log.v("Band10ValueMin", mesg);
                if(DataModel.Band10ValueMinIsChanged >= 1)
                {
                    Band10ValueMin.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Band10ValueMinb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Band10ValueMax
        final EditText Band10ValueMax = (EditText) findViewById(R.id.Band10ValueMax);
        Band10ValueMax.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Band10ValueMax", mesg,1);
                Log.v("Band10ValueMax", mesg);
                if(DataModel.Band10ValueMaxIsChanged >= 1)
                {
                    Band10ValueMax.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Band10ValueMaxb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //Band20After
        final EditText Band20After = (EditText) findViewById(R.id.Band20After);
        Band20After.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {

                String mesg = s.toString();
                DataModel.AddData("Band20After", mesg,1);
                Log.v("Band20After", mesg);
                if(DataModel.Band20AfterIsChanged >= 1)
                {
                    Band20After.setTextColor(Color.parseColor("#ee8033") );
                }
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                DataModel.Band20Afterb = s.toString();
            }
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });

        //
        //
        //
    }


    //Spinner Freq
    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        String sSelected=parent.getItemAtPosition(position).toString();
        //Toast.makeText(this,sSelected,Toast.LENGTH_SHORT).show();
        //RequestChangeSettingsFREQ=4432
        String sendmsg = "RequestChangeSettingsFREQ=";
        sendmsg += sSelected;
        if(sSelected.equals("value not loaded") == false)
            DataModel.AddData("FREQ", sSelected,1);
        Log.v("CounterState FREQIsC ", Integer.toString(DataModel.FREQIsChanged)  );
        if(DataModel.FREQIsChanged >= 1)
        {
            mySpinner.setBackgroundColor(Color.parseColor("#ee8033"));
           // SendUDP(sendmsg);
        }

    }


    @Override
    public void onNothingSelected(AdapterView<?> parent) {

    }

    private void setStatusText(final String value){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                TextStatusShow.setText(value);
                TextStatusShow.setTextColor(Color.parseColor("#00FF00" ) );
                //setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
        });
    }


    private DatagramSocket ds = null;

    @Override
    public void onBackPressed() {
        if (ds != null)
        {
            ds.close();
        }
        finish();
        return;
    }

    private void processPacket(final String msg)
    {
        Thread processthread = new Thread(new Runnable() {

            @Override
            public void run() {
                updateUI(msg);
            }

        });
        processthread.start();
    }

    private void startServerSocket() {

        Thread thread = new Thread(new Runnable() {

            private String stringData = null;

            @Override
            public void run() {

                byte[] msg = new byte[1000];
                DatagramPacket dp = new DatagramPacket(msg, msg.length);
                InetAddress ip = null;
                InetAddress tmp;

                try {
                    ds = new DatagramSocket(5115);
                    ds.setReceiveBufferSize(64*1024);
                    //ds.setSoTimeout(50000);
                    while(true)
                    {
                        ds.receive(dp);

                        if(IPstr == null) {
                            ip = dp.getAddress();

                            IPstr = new String();
                            IPstr = ip.toString().replace("/", "");

                            String status = "Ground IP: ";
                            status += IPstr;

                            RequestSettingsAll();

                            //processPacket(stringData);

                            try
                            {
                                setStatusText(status);
                                //TextView ts = (TextView) findViewById(R.id.TextStatusShow);
                                //ts.setText(status);
                                //ts.setHintTextColor(Color.parseColor("#00FF00" ) );
                            }
                            catch (Exception e)
                            {
                                e.printStackTrace();
                                Toast.makeText(getApplicationContext(), "Sorry, there was an Error", Toast.LENGTH_LONG).show();
                            }



                        }

                        tmp = dp.getAddress();
                        String tmpstr = tmp.toString().replace("/","");
                        if(tmpstr.equals(IPstr) == false && IPstr != null)
                        {
                            //refresh ip with new value
                            //ip = tmp;
                            String status = "Ground IP: " + tmpstr;
                            IPstr = tmpstr;

                            try
                            {
                                setStatusText(status);

                            }
                            catch (Exception e)
                            {
                                e.printStackTrace();
                                Toast.makeText(getApplicationContext(), "Sorry, there was an Error", Toast.LENGTH_LONG).show();
                            }
                        }

                        //InetAddress ia = dp.getAddress();

                        stringData = new String(msg, 0, dp.getLength());
                        processPacket(stringData);

                    }


                }
                catch (IOException e)
                {
                    e.printStackTrace();
                    Toast.makeText(getApplicationContext(), "Sorry, there was an Error", Toast.LENGTH_LONG).show();
                }
                finally
                {
                    if (ds != null)
                    {
                        ds.close();
                    }
                }
            }

        });
        thread.start();
    }

    private void PackMessageAndSend(String Variable, String Data)
    {
        if(Data.equals("value not loaded") == false)
        {
            //RequestChangeSettingsFREQ=4432
            String sendmsg = "RequestChangeSettings";
            sendmsg +=Variable;
            sendmsg += "=";
            sendmsg += Data;
            SendUDP(sendmsg);
            try
            {
                Thread.sleep(50);
            }
            catch(InterruptedException ex)
            {
                ex.printStackTrace();
                Toast.makeText(getApplicationContext(), "Sorry, there was an Error", Toast.LENGTH_LONG).show();
            }
        }
    }

    private void PackMessageAndSendOSD(String Variable, String Data)
    {
        if(Data.equals("value not loaded") == false)
        {
            //RequestChangeSettingsFREQ=4432
            String sendmsg = "RequestChangeOSD";
            sendmsg +=Variable;
            sendmsg += "=";
            sendmsg += Data;
            SendUDP(sendmsg);
            try
            {
                Thread.sleep(50);
            }
            catch(InterruptedException ex)
            {
                ex.printStackTrace();
                Toast.makeText(getApplicationContext(), "Sorry, there was an Error", Toast.LENGTH_LONG).show();
            }
        }
    }

    private void PackMessageAndSendTxPower(String Variable, String Data)
    {
        if(Data.equals("value not loaded") == false)
        {
            //RequestChangeTxPowertxpowerA=4432
            String sendmsg = "RequestChangeTxPower";
            sendmsg +=Variable;
            sendmsg += "=";
            sendmsg += Data;
            SendUDP(sendmsg);
            try
            {
                Thread.sleep(50);
            }
            catch(InterruptedException ex)
            {
                ex.printStackTrace();
                Toast.makeText(getApplicationContext(), "Sorry, there was an Error", Toast.LENGTH_LONG).show();
            }
        }
    }


    private void PackMessageAndSendJoystick(String Variable, String Data)
    {
        if(Data.equals("value not loaded") == false)
        {
            //RequestChangeSettingsFREQ=4432
            String sendmsg = "RequestChangeJoystick";
            sendmsg +=Variable;
            sendmsg += "=";
            sendmsg += Data;
            SendUDP(sendmsg);
            try
            {
                Thread.sleep(50);
            }
            catch(InterruptedException ex)
            {
                ex.printStackTrace();
                Toast.makeText(getApplicationContext(), "Sorry, there was an Error", Toast.LENGTH_LONG).show();
            }
        }
    }

    private void SendUDP(final String message)
    {
        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {

                try
                {
                    if(IPstr != null)
                    {
                        DatagramSocket dsSend = new DatagramSocket();
                        InetAddress IPRemote = InetAddress.getByName(IPstr);
                        String msg = message;
                        DatagramPacket dpSend = new DatagramPacket(msg.getBytes(), msg.length(), IPRemote, 1011);
                        dsSend.send(dpSend);
                        dsSend.close();
                    }

                }
                catch (IOException e)
                {
                    e.printStackTrace();
                    Toast.makeText(getApplicationContext(), "Sorry, there was an Error", Toast.LENGTH_LONG).show();
                }
                finally {

                }


            }

        });
        thread.start();
    }

    private void RequestSaveSettings()
    {
        //PackMessageAndSend();
        if(DataModel.CopterIsChanged >= 1) { PackMessageAndSendOSD("Copter", DataModel.Copter); }
        if(DataModel.ImperialIsChanged >= 1) { PackMessageAndSendOSD("Imperial", DataModel.Imperial); }
        if(DataModel.UPDATE_NTH_TIMEIsChanged >= 1) { PackMessageAndSendJoystick("UPDATE_NTH_TIME", DataModel.UPDATE_NTH_TIME); }
        if(DataModel.txpowerAIsChanged >= 1) { PackMessageAndSendTxPower("txpowerA", DataModel.txpowerA); }
        if(DataModel.txpowerRIsChanged >= 1) { PackMessageAndSendTxPower("txpowerR", DataModel.txpowerR); }




        if(DataModel.RemoteSettingsEnabledIsChanged >= 1) { PackMessageAndSend("RemoteSettingsEnabled", DataModel.RemoteSettingsEnabled); }
        if(DataModel.DefaultAudioOutIsChanged >= 1) { PackMessageAndSend("DefaultAudioOut", DataModel.DefaultAudioOut); }
        if(DataModel.IsAudioTransferEnabledIsChanged >= 1) { PackMessageAndSend("IsAudioTransferEnabled", DataModel.IsAudioTransferEnabled); }

        if(DataModel.FC_RC_BAUDRATEIsChanged >= 1) { PackMessageAndSend("FC_RC_BAUDRATE", DataModel.FC_RC_BAUDRATE); }
        if(DataModel.FC_RC_SERIALPORTIsChanged >= 1) { PackMessageAndSend("FC_RC_SERIALPORT", DataModel.FC_RC_SERIALPORT); }
        if(DataModel.FC_TELEMETRY_BAUDRATEIsChanged >= 1) { PackMessageAndSend("FC_TELEMETRY_BAUDRATE", DataModel.FC_TELEMETRY_BAUDRATE); }
        if(DataModel.FC_TELEMETRY_SERIALPORTIsChanged >= 1) { PackMessageAndSend("FC_TELEMETRY_SERIALPORT", DataModel.FC_TELEMETRY_SERIALPORT); }
        if(DataModel.FC_MSP_BAUDRATEIsChanged >= 1) { PackMessageAndSend("FC_MSP_BAUDRATE", DataModel.FC_MSP_BAUDRATE); }
        if(DataModel.FC_MSP_SERIALPORTIsChanged >= 1) { PackMessageAndSend("FC_MSP_SERIALPORT", DataModel.FC_MSP_SERIALPORT); }


        if(DataModel.FREQIsChanged >= 1) { PackMessageAndSend("FREQ", DataModel.FREQ); }
        if(DataModel.FPSIsChanged >= 1) { PackMessageAndSend("FPS", DataModel.FPS); }
        if(DataModel.DATARATEIsChanged >= 1) { PackMessageAndSend("DATARATE", DataModel.DATARATE); }
        if(DataModel.VIDEO_BLOCKSIsChanged >= 1) { PackMessageAndSend("VIDEO_BLOCKS", DataModel.VIDEO_BLOCKS); }

        if(DataModel.VIDEO_FECSIsChanged >= 1) { PackMessageAndSend("VIDEO_FECS", DataModel.VIDEO_FECS); }
        if(DataModel.VIDEO_BLOCKLENGTHIsChanged >= 1) { PackMessageAndSend("VIDEO_BLOCKLENGTH", DataModel.VIDEO_BLOCKLENGTH); }
        if(DataModel.VIDEO_BITRATEIsChanged >= 1) { PackMessageAndSend("VIDEO_BITRATE", DataModel.VIDEO_BITRATE); }
        if(DataModel.BITRATE_PERCENTIsChanged >= 1) { PackMessageAndSend("BITRATE_PERCENT", DataModel.BITRATE_PERCENT); }
        if(DataModel.WIDTHIsChanged >= 1) { PackMessageAndSend("WIDTH", DataModel.WIDTH); }
        if(DataModel.HEIGHTIsChanged >= 1) { PackMessageAndSend("HEIGHT", DataModel.HEIGHT); }
        if(DataModel.FPSIsChanged >= 1) { PackMessageAndSend("FPS", DataModel.FPS); }
        if(DataModel.KEYFRAMERATEIsChanged >= 1) { PackMessageAndSend("KEYFRAMERATE", DataModel.KEYFRAMERATE); }
        if(DataModel.EXTRAPARAMSIsChanged >= 1) { PackMessageAndSend("EXTRAPARAMS", DataModel.EXTRAPARAMS); }

        //add
        if(DataModel.FREQSCANIsChanged >= 1) { PackMessageAndSend("FREQSCAN", DataModel.FREQSCAN); }
        if(DataModel.TXMODEIsChanged >= 1) { PackMessageAndSend("TXMODE", DataModel.TXMODE); }
        if(DataModel.TELEMETRY_TRANSMISSIONIsChanged >= 1) { PackMessageAndSend("TELEMETRY_TRANSMISSION", DataModel.TELEMETRY_TRANSMISSION); }
        if(DataModel.TELEMETRY_UPLINKIsChanged >= 1) { PackMessageAndSend("TELEMETRY_UPLINK", DataModel.TELEMETRY_UPLINK); }
        if(DataModel.RCIsChanged >= 1) { PackMessageAndSend("RC", DataModel.RC); }
        if(DataModel.CTS_PROTECTIONIsChanged >= 1) { PackMessageAndSend("CTS_PROTECTION", DataModel.CTS_PROTECTION);}
        if(DataModel.WIFI_HOTSPOTIsChanged >= 1) { PackMessageAndSend("WIFI_HOTSPOT", DataModel.WIFI_HOTSPOT); }
        if(DataModel.WIFI_HOTSPOT_NICIsChanged >= 1) { PackMessageAndSend("WIFI_HOTSPOT_NIC", DataModel.WIFI_HOTSPOT_NIC); }
        if(DataModel.ETHERNET_HOTSPOTIsChanged >= 1) { PackMessageAndSend("ETHERNET_HOTSPOT", DataModel.ETHERNET_HOTSPOT); }
        if(DataModel.ENABLE_SCREENSHOTSIsChanged >= 1) { PackMessageAndSend("ENABLE_SCREENSHOTS", DataModel.ENABLE_SCREENSHOTS); }
        if(DataModel.FORWARD_STREAMIsChanged >= 1) { PackMessageAndSend("FORWARD_STREAM", DataModel.FORWARD_STREAM); }
        if(DataModel.VIDEO_UDP_PORTIsChanged >= 1) { PackMessageAndSend("VIDEO_UDP_PORT", DataModel.VIDEO_UDP_PORT); }

        //add Camera Control
        if(DataModel.IsCamera1EnabledIsChanged >= 1) { PackMessageAndSend("IsCamera1Enabled", DataModel.IsCamera1Enabled); }
        if(DataModel.IsCamera2EnabledIsChanged >= 1) { PackMessageAndSend("IsCamera2Enabled", DataModel.IsCamera2Enabled); }
        if(DataModel.IsCamera3EnabledIsChanged >= 1) { PackMessageAndSend("IsCamera3Enabled", DataModel.IsCamera3Enabled); }
        if(DataModel.IsCamera4EnabledIsChanged >= 1) { PackMessageAndSend("IsCamera4Enabled", DataModel.IsCamera4Enabled); }
        if(DataModel.DefaultCameraIdIsChanged >= 1) { PackMessageAndSend("DefaultCameraId", DataModel.DefaultCameraId); }
        if(DataModel.ChannelToListenIsChanged >= 1) { PackMessageAndSend("ChannelToListen", DataModel.ChannelToListen); }
        if(DataModel.Camera1ValueMinIsChanged >= 1) { PackMessageAndSend("Camera1ValueMin", DataModel.Camera1ValueMin); }
        if(DataModel.Camera1ValueMaxIsChanged >= 1) { PackMessageAndSend("Camera1ValueMax", DataModel.Camera1ValueMax); }
        if(DataModel.Camera2ValueMinIsChanged >= 1) { PackMessageAndSend("Camera2ValueMin", DataModel.Camera2ValueMin); }
        if(DataModel.Camera2ValueMaxIsChanged >= 1) { PackMessageAndSend("Camera2ValueMax", DataModel.Camera2ValueMax); }
        if(DataModel.Camera3ValueMinIsChanged >= 1) { PackMessageAndSend("Camera3ValueMin", DataModel.Camera3ValueMin); }
        if(DataModel.Camera3ValueMaxIsChanged >= 1) { PackMessageAndSend("Camera3ValueMax", DataModel.Camera3ValueMax); }
        if(DataModel.Camera4ValueMinIsChanged >= 1) { PackMessageAndSend("Camera4ValueMin", DataModel.Camera4ValueMin); }
        if(DataModel.Camera4ValueMaxIsChanged >= 1) { PackMessageAndSend("Camera4ValueMax", DataModel.Camera4ValueMax); }

        //Requested Menu Selections
        if(DataModel.CELLSIsChanged >= 1) { PackMessageAndSendOSD("CELLS", DataModel.CELLS); }
        if(DataModel.EncryptionOrRangeIsChanged >= 1) { PackMessageAndSend("EncryptionOrRange", DataModel.EncryptionOrRange); }
        if(DataModel.IsBandSwicherEnabledIsChanged >= 1) { PackMessageAndSend("IsBandSwicherEnabled", DataModel.IsBandSwicherEnabled); }
        if(DataModel.BandwidthIsChanged >= 1) { PackMessageAndSend("Bandwidth", DataModel.Bandwidth); }
        if(DataModel.UplinkSpeedIsChanged >= 1) { PackMessageAndSend("UplinkSpeed", DataModel.UplinkSpeed); }
        if(DataModel.ChannelToListen2IsChanged >= 1) { PackMessageAndSend("ChannelToListen2", DataModel.ChannelToListen2); }
        if(DataModel.PrimaryCardMACIsChanged >= 1) { PackMessageAndSend("PrimaryCardMAC", DataModel.PrimaryCardMAC); }
        if(DataModel.SlaveCardMACIsChanged >= 1) { PackMessageAndSend("SlaveCardMAC", DataModel.SlaveCardMAC); }
        if(DataModel.Band5BelowIsChanged >= 1) { PackMessageAndSend("Band5Below", DataModel.Band5Below); }
        if(DataModel.Band10ValueMinIsChanged >= 1) { PackMessageAndSend("Band10ValueMin", DataModel.Band10ValueMin); }
        if(DataModel.Band10ValueMaxIsChanged >= 1) { PackMessageAndSend("Band10ValueMax", DataModel.Band10ValueMax); }
        if(DataModel.Band20AfterIsChanged >= 1) { PackMessageAndSend("Band20After", DataModel.Band20After); }
    }

    private void RequestSettingsAll()
    {
        Thread thread = new Thread(new Runnable() {

            @Override
            public void run() {

                try
                {
                    if(IPstr != null)
                    {
                        //DataModel.DecCounter(1);
                        DatagramSocket dsSend = new DatagramSocket();
                        String str = "RequestAllSettings";
                        InetAddress IPRemote = InetAddress.getByName(IPstr);

                        DatagramPacket dpSend = new DatagramPacket(str.getBytes(), str.length(), IPRemote, 1011);
                        dsSend.send(dpSend);
                        dsSend.close();
                    }
                    //      dsSend.close();
                }
                catch (IOException e)
                {
                    e.printStackTrace();
                    Toast.makeText(getApplicationContext(), "Sorry, there was an Error", Toast.LENGTH_LONG).show();
                }


            }

        });
        thread.start();



    }

    private void updateUI(final String stringData) {

        handler.post(new Runnable() {
            @Override
            public void run() {

                if (stringData.length() >= 13)
                {

                    String[] parts = stringData.split("ConfigResp");
                    String part1 = parts[0]; // 004
                    String part2 = parts[1]; // 034556

                    String ValData = parts[1];
                    String ValDataArr[] = ValData.split("=");
                    String Val = ValDataArr[0];
                    String Data = ValDataArr[1];

                    CheckUIComponentToRefresh(Val, Data);
                    if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
                    {
                        DataModel.AddData(Val,Data,0);
                    }
                }

            }
        });
    }
    
    String ColorsBoth = "#00FF00";
    String ColorsAirOnly = "#0000FF";
    String ColorsGroundOnly = "#FF0000";

    private void CheckUIComponentToRefresh(String Val, String Data)
    {
        if(Val.equals("Saved") == true)
        {
            String str = Data;
            str += " Saved";
            TextStatusShow.setText(str);
        }

        if(Val.equals("FC_RC_BAUDRATE") == true)
        {
            //EditText FC_RC_BAUDRATE = (EditText) findViewById(R.id.EditFC_RC_BAUDRATE);
            //FC_RC_BAUDRATE.setText(Data);
            //mySpinner.setSelection(adapter.getPosition(Data));
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FC_RC_BAUDRATEAirAck = 1;
                SpinnerFC_RC_BAUDRATE.setBackgroundColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FC_RC_BAUDRATEGroundAck = 1;
                SpinnerFC_RC_BAUDRATE.setBackgroundColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.FC_RC_BAUDRATEGroundAck == 1 && DataModel.FC_RC_BAUDRATEAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                SpinnerFC_RC_BAUDRATE.setBackgroundColor(Color.parseColor(ColorsBoth) );
                DataModel.FC_RC_BAUDRATEGroundAck = 0;
                DataModel.FC_RC_BAUDRATEAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                String selectedNow = SpinnerFC_RC_BAUDRATE.getSelectedItem().toString();
                if(selectedNow.equals(Data) == true)
                    DataModel.FC_RC_BAUDRATEIsChanged = -1;
                if(selectedNow.equals("value not loaded") == true)
                    DataModel.FC_RC_BAUDRATEIsChanged = -1;
                SpinnerFC_RC_BAUDRATE.setSelection(((ArrayAdapter)SpinnerFC_RC_BAUDRATE.getAdapter()).getPosition(Data));
            }
        }


        if(Val.equals("FC_TELEMETRY_BAUDRATE") == true)
        {
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FC_TELEMETRY_BAUDRATEAirAck = 1;
                SpinnerFC_TELEMETRY_BAUDRATE.setBackgroundColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FC_TELEMETRY_BAUDRATEGroundAck = 1;
                SpinnerFC_TELEMETRY_BAUDRATE.setBackgroundColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.FC_TELEMETRY_BAUDRATEGroundAck == 1 && DataModel.FC_TELEMETRY_BAUDRATEAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                SpinnerFC_TELEMETRY_BAUDRATE.setBackgroundColor(Color.parseColor(ColorsBoth) );
                DataModel.FC_TELEMETRY_BAUDRATEGroundAck = 0;
                DataModel.FC_TELEMETRY_BAUDRATEAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                String selectedNow = SpinnerFC_TELEMETRY_BAUDRATE.getSelectedItem().toString();
                if(selectedNow.equals(Data) == true)
                    DataModel.FC_TELEMETRY_BAUDRATEIsChanged = -1;
                if(selectedNow.equals("value not loaded") == true)
                    DataModel.FC_TELEMETRY_BAUDRATEIsChanged = -1;
                SpinnerFC_TELEMETRY_BAUDRATE.setSelection(((ArrayAdapter)SpinnerFC_TELEMETRY_BAUDRATE.getAdapter()).getPosition(Data));
            }
        }


        if(Val.equals("FC_MSP_BAUDRATE") == true)
        {
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FC_MSP_BAUDRATEAirAck = 1;
                SpinnerFC_MSP_BAUDRATE.setBackgroundColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FC_MSP_BAUDRATEGroundAck = 1;
                SpinnerFC_MSP_BAUDRATE.setBackgroundColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.FC_MSP_BAUDRATEGroundAck == 1 && DataModel.FC_MSP_BAUDRATEAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                SpinnerFC_MSP_BAUDRATE.setBackgroundColor(Color.parseColor(ColorsBoth) );
                DataModel.FC_MSP_BAUDRATEGroundAck = 0;
                DataModel.FC_MSP_BAUDRATEAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                String selectedNow = SpinnerFC_MSP_BAUDRATE.getSelectedItem().toString();
                if(selectedNow.equals(Data) == true)
                    DataModel.FC_MSP_BAUDRATEIsChanged = -1;
                if(selectedNow.equals("value not loaded") == true)
                    DataModel.FC_MSP_BAUDRATEIsChanged = -1;
                SpinnerFC_MSP_BAUDRATE.setSelection(((ArrayAdapter)SpinnerFC_MSP_BAUDRATE.getAdapter()).getPosition(Data));
            }
        }


        if(Val.equals("FREQ") == true)
        {
            //EditText freq = (EditText) findViewById(R.id.EditFreq);
            //freq.setText(Data);
            //mySpinner.setSelection(adapter.getPosition(Data));
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FREQAirAck = 1;
                mySpinner.setBackgroundColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FREQGroundAck = 1;
                mySpinner.setBackgroundColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.FREQGroundAck == 1 && DataModel.FREQAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                mySpinner.setBackgroundColor(Color.parseColor(ColorsBoth) );
                DataModel.FREQGroundAck = 0;
                DataModel.FREQAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                String selectedNow = mySpinner.getSelectedItem().toString();
                if(selectedNow.equals(Data) == true)
                    DataModel.FREQIsChanged = -1;
                if(selectedNow.equals("value not loaded") == true)
                    DataModel.FREQIsChanged = -1;
                mySpinner.setSelection(((ArrayAdapter)mySpinner.getAdapter()).getPosition(Data));
            }
        }

        if(Val.equals("FC_RC_SERIALPORT") == true)
    {
        //Toast.makeText(getApplicationContext(), "FC_RC_SERIALPORT in "+ Data, Toast.LENGTH_LONG).show();
        EditText FC_RC_SERIALPORT = (EditText) findViewById(R.id.FC_RC_SERIALPORT);

        //if Request values - set color black
        //if Air or Ground - incrise counter by one. and change colore to green or yelow
        //if counter == 2 set colore to black + set counter = 0
        //ConfigRespBITRATE_PERCENT=SavedAir
        //ConfigRespBITRATE_PERCENT=SavedGround
        //ConfigRespWIDTH=1280
        if(Data.equals("SavedAir") == true)
        {
            //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
            DataModel.FC_RC_SERIALPORTAirAck = 1;
            FC_RC_SERIALPORT.setTextColor(Color.parseColor(ColorsAirOnly) );
        }
        if(Data.equals("SavedGround") == true)
        {
            //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
            DataModel.FC_RC_SERIALPORTGroundAck = 1;
            FC_RC_SERIALPORT.setTextColor(Color.parseColor(ColorsGroundOnly) );
        }
        if(DataModel.FC_RC_SERIALPORTGroundAck == 1 && DataModel.FC_RC_SERIALPORTAirAck == 1)
        {
            Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
            FC_RC_SERIALPORT.setTextColor(Color.parseColor(ColorsBoth) );
            DataModel.FC_RC_SERIALPORTGroundAck = 0;
            DataModel.FC_RC_SERIALPORTAirAck = 0;
        }
        if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
        {
            //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
            FC_RC_SERIALPORT.setText(Data);
        }
    }


        if(Val.equals("DefaultAudioOut") == true)
        {
            //Toast.makeText(getApplicationContext(), "DefaultAudioOut in "+ Data, Toast.LENGTH_LONG).show();
            EditText DefaultAudioOut = (EditText) findViewById(R.id.DefaultAudioOut);

            //if Request values - set color black
            //if Air or Ground - incrise counter by one. and change colore to green or yelow
            //if counter == 2 set colore to black + set counter = 0
            //ConfigRespBITRATE_PERCENT=SavedAir
            //ConfigRespBITRATE_PERCENT=SavedGround
            //ConfigRespWIDTH=1280
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.DefaultAudioOutAirAck = 1;
                DefaultAudioOut.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.DefaultAudioOutGroundAck = 1;
                DefaultAudioOut.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.DefaultAudioOutGroundAck == 1 && DataModel.DefaultAudioOutAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                DefaultAudioOut.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.DefaultAudioOutGroundAck = 0;
                DataModel.DefaultAudioOutAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                DefaultAudioOut.setText(Data);
            }
        }




        if(Val.equals("RemoteSettingsEnabled") == true)
        {
            //Toast.makeText(getApplicationContext(), "DefaultAudioOut in "+ Data, Toast.LENGTH_LONG).show();
            EditText RemoteSettingsEnabled = (EditText) findViewById(R.id.RemoteSettingsEnabled);

            //if Request values - set color black
            //if Air or Ground - incrise counter by one. and change colore to green or yelow
            //if counter == 2 set colore to black + set counter = 0
            //ConfigRespBITRATE_PERCENT=SavedAir
            //ConfigRespBITRATE_PERCENT=SavedGround
            //ConfigRespWIDTH=1280
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.RemoteSettingsEnabledAirAck = 1;
                RemoteSettingsEnabled.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.RemoteSettingsEnabledGroundAck = 1;
                RemoteSettingsEnabled.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.RemoteSettingsEnabledGroundAck == 1 && DataModel.RemoteSettingsEnabledAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                RemoteSettingsEnabled.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.RemoteSettingsEnabledGroundAck = 0;
                DataModel.RemoteSettingsEnabledAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                RemoteSettingsEnabled.setText(Data);
            }
        }



        if(Val.equals("IsAudioTransferEnabled") == true)
        {
            //Toast.makeText(getApplicationContext(), "IsAudioTransferEnabled in "+ Data, Toast.LENGTH_LONG).show();
            EditText IsAudioTransferEnabled = (EditText) findViewById(R.id.IsAudioTransferEnabled);

            //if Request values - set color black
            //if Air or Ground - incrise counter by one. and change colore to green or yelow
            //if counter == 2 set colore to black + set counter = 0
            //ConfigRespBITRATE_PERCENT=SavedAir
            //ConfigRespBITRATE_PERCENT=SavedGround
            //ConfigRespWIDTH=1280
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.IsAudioTransferEnabledAirAck = 1;
                IsAudioTransferEnabled.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.IsAudioTransferEnabledGroundAck = 1;
                IsAudioTransferEnabled.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.IsAudioTransferEnabledGroundAck == 1 && DataModel.IsAudioTransferEnabledAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                IsAudioTransferEnabled.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.IsAudioTransferEnabledGroundAck = 0;
                DataModel.IsAudioTransferEnabledAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                IsAudioTransferEnabled.setText(Data);
            }
        }

        if(Val.equals("txpowerA") == true)
        {
            //Toast.makeText(getApplicationContext(), "txpowerA in "+ Data, Toast.LENGTH_LONG).show();
            EditText txpowerA = (EditText) findViewById(R.id.txpowerA);

            //if Request values - set color black
            //if Air or Ground - incrise counter by one. and change colore to green or yelow
            //if counter == 2 set colore to black + set counter = 0
            //ConfigRespBITRATE_PERCENT=SavedAir
            //ConfigRespBITRATE_PERCENT=SavedGround
            //ConfigRespWIDTH=1280
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.txpowerAAirAck = 1;
                txpowerA.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.txpowerAGroundAck = 1;
                txpowerA.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.txpowerAGroundAck == 1 && DataModel.txpowerAAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                txpowerA.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.txpowerAGroundAck = 0;
                DataModel.txpowerAAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                txpowerA.setText(Data);
            }
        }

        if(Val.equals("txpowerR") == true)
        {
            //Toast.makeText(getApplicationContext(), "txpowerR in "+ Data, Toast.LENGTH_LONG).show();
            EditText txpowerR = (EditText) findViewById(R.id.txpowerR);

            //if Request values - set color black
            //if Air or Ground - incrise counter by one. and change colore to green or yelow
            //if counter == 2 set colore to black + set counter = 0
            //ConfigRespBITRATE_PERCENT=SavedAir
            //ConfigRespBITRATE_PERCENT=SavedGround
            //ConfigRespWIDTH=1280
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.txpowerRAirAck = 1;
                txpowerR.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.txpowerRGroundAck = 1;
                txpowerR.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.txpowerRGroundAck == 1 && DataModel.txpowerRAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                txpowerR.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.txpowerRGroundAck = 0;
                DataModel.txpowerRAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                txpowerR.setText(Data);
            }
        }


        if(Val.equals("FC_TELEMETRY_SERIALPORT") == true)
    {
        //Toast.makeText(getApplicationContext(), "FC_TELEMETRY_SERIALPORT in "+ Data, Toast.LENGTH_LONG).show();
        EditText FC_TELEMETRY_SERIALPORT = (EditText) findViewById(R.id.FC_TELEMETRY_SERIALPORT);

        //if Request values - set color black
        //if Air or Ground - incrise counter by one. and change colore to green or yelow
        //if counter == 2 set colore to black + set counter = 0
        //ConfigRespBITRATE_PERCENT=SavedAir
        //ConfigRespBITRATE_PERCENT=SavedGround
        //ConfigRespWIDTH=1280
        if(Data.equals("SavedAir") == true)
        {
            //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
            DataModel.FC_TELEMETRY_SERIALPORTAirAck = 1;
            FC_TELEMETRY_SERIALPORT.setTextColor(Color.parseColor(ColorsAirOnly) );
        }
        if(Data.equals("SavedGround") == true)
        {
            //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
            DataModel.FC_TELEMETRY_SERIALPORTGroundAck = 1;
            FC_TELEMETRY_SERIALPORT.setTextColor(Color.parseColor(ColorsGroundOnly) );
        }
        if(DataModel.FC_TELEMETRY_SERIALPORTGroundAck == 1 && DataModel.FC_TELEMETRY_SERIALPORTAirAck == 1)
        {
            Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
            FC_TELEMETRY_SERIALPORT.setTextColor(Color.parseColor(ColorsBoth) );
            DataModel.FC_TELEMETRY_SERIALPORTGroundAck = 0;
            DataModel.FC_TELEMETRY_SERIALPORTAirAck = 0;
        }
        if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
        {
            //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
            FC_TELEMETRY_SERIALPORT.setText(Data);
        }

    }


        if(Val.equals("FC_MSP_SERIALPORT") == true)
        {
            //Toast.makeText(getApplicationContext(), "FC_MSP_SERIALPORT in "+ Data, Toast.LENGTH_LONG).show();
            EditText FC_MSP_SERIALPORT = (EditText) findViewById(R.id.FC_MSP_SERIALPORT);

            //if Request values - set color black
            //if Air or Ground - incrise counter by one. and change colore to green or yelow
            //if counter == 2 set colore to black + set counter = 0
            //ConfigRespBITRATE_PERCENT=SavedAir
            //ConfigRespBITRATE_PERCENT=SavedGround
            //ConfigRespWIDTH=1280
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FC_MSP_SERIALPORTAirAck = 1;
                FC_MSP_SERIALPORT.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FC_MSP_SERIALPORTGroundAck = 1;
                FC_MSP_SERIALPORT.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.FC_MSP_SERIALPORTGroundAck == 1 && DataModel.FC_MSP_SERIALPORTAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                FC_MSP_SERIALPORT.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.FC_MSP_SERIALPORTGroundAck = 0;
                DataModel.FC_MSP_SERIALPORTAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                FC_MSP_SERIALPORT.setText(Data);
            }

        }


        if(Val.equals("UPDATE_NTH_TIME") == true)
        {
            //Toast.makeText(getApplicationContext(), "UPDATE_NTH_TIME in "+ Data, Toast.LENGTH_LONG).show();
            EditText UPDATE_NTH_TIME = (EditText) findViewById(R.id.UPDATE_NTH_TIME);

            //if Request values - set color black
            //if Air or Ground - incrise counter by one. and change colore to green or yelow
            //if counter == 2 set colore to black + set counter = 0
            //ConfigRespBITRATE_PERCENT=SavedAir
            //ConfigRespBITRATE_PERCENT=SavedGround
            //ConfigRespWIDTH=1280
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.UPDATE_NTH_TIMEAirAck = 1;
                UPDATE_NTH_TIME.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.UPDATE_NTH_TIMEGroundAck = 1;
                UPDATE_NTH_TIME.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.UPDATE_NTH_TIMEGroundAck == 1 && DataModel.UPDATE_NTH_TIMEAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                UPDATE_NTH_TIME.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.UPDATE_NTH_TIMEGroundAck = 0;
                DataModel.UPDATE_NTH_TIMEAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                UPDATE_NTH_TIME.setText(Data);
            }

        }

        if(Val.equals("Copter") == true)
        {
            //EditText freq = (EditText) findViewById(R.id.EditFreq);
            //freq.setText(Data);
            //mySpinner.setSelection(adapter.getPosition(Data));
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.CopterAirAck = 1;
                SpinnerCopter.setBackgroundColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.CopterGroundAck = 1;
                SpinnerCopter.setBackgroundColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.CopterGroundAck == 1 && DataModel.CopterAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                SpinnerCopter.setBackgroundColor(Color.parseColor(ColorsBoth) );
                DataModel.CopterGroundAck = 0;
                DataModel.CopterAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                String selectedNow = SpinnerCopter.getSelectedItem().toString();
                if(selectedNow.equals(Data) == true)
                    DataModel.CopterIsChanged = -1;
                if(selectedNow.equals("value not loaded") == true)
                    DataModel.CopterIsChanged = -1;
                SpinnerCopter.setSelection(((ArrayAdapter)SpinnerCopter.getAdapter()).getPosition(Data));
            }
        }


        if(Val.equals("Imperial") == true)
        {
            //EditText freq = (EditText) findViewById(R.id.EditFreq);
            //freq.setText(Data);
            //mySpinner.setSelection(adapter.getPosition(Data));
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.ImperialAirAck = 1;
                SpinnerImperial.setBackgroundColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.ImperialGroundAck = 1;
                SpinnerImperial.setBackgroundColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.ImperialGroundAck == 1 && DataModel.ImperialAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                SpinnerImperial.setBackgroundColor(Color.parseColor(ColorsBoth) );
                DataModel.ImperialGroundAck = 0;
                DataModel.ImperialAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                String selectedNow = SpinnerImperial.getSelectedItem().toString();
                if(selectedNow.equals(Data) == true)
                    DataModel.ImperialIsChanged = -1;
                if(selectedNow.equals("value not loaded") == true)
                    DataModel.ImperialIsChanged = -1;
                SpinnerImperial.setSelection(((ArrayAdapter)SpinnerImperial.getAdapter()).getPosition(Data));
            }
        }


        if(Val.equals("DATARATE") == true)
        {
            //Toast.makeText(getApplicationContext(), "DATARATE in "+ Data, Toast.LENGTH_LONG).show();
            EditText DATARATE = (EditText) findViewById(R.id.DATARATE);

            //if Request values - set color black
            //if Air or Ground - incrise counter by one. and change colore to green or yelow
            //if counter == 2 set colore to black + set counter = 0
            //ConfigRespBITRATE_PERCENT=SavedAir
            //ConfigRespBITRATE_PERCENT=SavedGround
            //ConfigRespWIDTH=1280
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.DATARATEAirAck = 1;
                DATARATE.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.DATARATEGroundAck = 1;
                DATARATE.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.DATARATEGroundAck == 1 && DataModel.DATARATEAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                DATARATE.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.DATARATEGroundAck = 0;
                DataModel.DATARATEAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                DATARATE.setText(Data);
            }

        }
        /////////////////////////////
        if(Val.equals("VIDEO_BLOCKS") == true)
        {
            EditText VIDEO_BLOCKS = (EditText) findViewById(R.id.VIDEO_BLOCKS);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.VIDEO_BLOCKSAirAck = 1;
                VIDEO_BLOCKS.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.VIDEO_BLOCKSGroundAck = 1;
                VIDEO_BLOCKS.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.VIDEO_BLOCKSGroundAck == 1 && DataModel.VIDEO_BLOCKSAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                VIDEO_BLOCKS.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.VIDEO_BLOCKSGroundAck = 0;
                DataModel.VIDEO_BLOCKSAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                VIDEO_BLOCKS.setText(Data);
            }
        }
        if(Val.equals("VIDEO_FECS") == true)
        {
            EditText VIDEO_FECS = (EditText) findViewById(R.id.VIDEO_FECS);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.VIDEO_FECSAirAck = 1;
                VIDEO_FECS.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.VIDEO_FECSGroundAck = 1;
                VIDEO_FECS.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.VIDEO_FECSGroundAck == 1 && DataModel.VIDEO_FECSAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                VIDEO_FECS.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.VIDEO_FECSGroundAck = 0;
                DataModel.VIDEO_FECSAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                VIDEO_FECS.setText(Data);
            }
        }
        if(Val.equals("VIDEO_BLOCKLENGTH") == true)
        {
            EditText VIDEO_BLOCKLENGTH = (EditText) findViewById(R.id.VIDEO_BLOCKLENGTH);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.VIDEO_BLOCKLENGTHAirAck = 1;
                VIDEO_BLOCKLENGTH.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.VIDEO_BLOCKLENGTHGroundAck = 1;
                VIDEO_BLOCKLENGTH.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.VIDEO_BLOCKLENGTHGroundAck == 1 && DataModel.VIDEO_BLOCKLENGTHAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                VIDEO_BLOCKLENGTH.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.VIDEO_BLOCKLENGTHGroundAck = 0;
                DataModel.VIDEO_BLOCKLENGTHAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                VIDEO_BLOCKLENGTH.setText(Data);
            }
        }
        //
        if(Val.equals("VIDEO_BITRATE") == true)
        {
            EditText VIDEO_BITRATE = (EditText) findViewById(R.id.VIDEO_BITRATE);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.VIDEO_BITRATEAirAck = 1;
                VIDEO_BITRATE.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.VIDEO_BITRATEGroundAck = 1;
                VIDEO_BITRATE.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.VIDEO_BITRATEGroundAck == 1 && DataModel.VIDEO_BITRATEAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                VIDEO_BITRATE.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.VIDEO_BITRATEGroundAck = 0;
                DataModel.VIDEO_BITRATEAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                VIDEO_BITRATE.setText(Data);
            }
        }
        if(Val.equals("BITRATE_PERCENT") == true)
        {
            EditText BITRATE_PERCENT = (EditText) findViewById(R.id.BITRATE_PERCENT);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.BITRATE_PERCENTAirAck = 1;
                BITRATE_PERCENT.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.BITRATE_PERCENTGroundAck = 1;
                BITRATE_PERCENT.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.BITRATE_PERCENTGroundAck == 1 && DataModel.BITRATE_PERCENTAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                BITRATE_PERCENT.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.BITRATE_PERCENTGroundAck = 0;
                DataModel.BITRATE_PERCENTAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                BITRATE_PERCENT.setText(Data);
            }
        }
        if(Val.equals("WIDTH") == true)
        {
            EditText WIDTH = (EditText) findViewById(R.id.WIDTH);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.WIDTHAirAck = 1;
                WIDTH.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.WIDTHGroundAck = 1;
                WIDTH.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.WIDTHGroundAck == 1 && DataModel.WIDTHAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                WIDTH.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.WIDTHGroundAck = 0;
                DataModel.WIDTHAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                WIDTH.setText(Data);
            }
        }
        if(Val.equals("HEIGHT") == true)
        {
            EditText HEIGHT = (EditText) findViewById(R.id.HEIGHT);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.HEIGHTAirAck = 1;
                HEIGHT.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.HEIGHTGroundAck = 1;
                HEIGHT.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.HEIGHTGroundAck == 1 && DataModel.HEIGHTAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                HEIGHT.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.HEIGHTGroundAck = 0;
                DataModel.HEIGHTAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                HEIGHT.setText(Data);
            }
        }
        if(Val.equals("EXTRAPARAMS") == true)
        {
            EditText EXTRAPARAMS = (EditText) findViewById(R.id.EXTRAPARAMS);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.EXTRAPARAMSAirAck = 1;
                EXTRAPARAMS.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.EXTRAPARAMSGroundAck = 1;
                EXTRAPARAMS.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.EXTRAPARAMSGroundAck == 1 && DataModel.EXTRAPARAMSAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                EXTRAPARAMS.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.EXTRAPARAMSGroundAck = 0;
                DataModel.EXTRAPARAMSAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                EXTRAPARAMS.setText(Data);
            }

        }
        if(Val.equals("KEYFRAMERATE") == true)
        {
            EditText KEYFRAMERATE = (EditText) findViewById(R.id.KEYFRAMERATE);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.KEYFRAMERATEAirAck = 1;
                KEYFRAMERATE.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.KEYFRAMERATEGroundAck = 1;
                KEYFRAMERATE.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.KEYFRAMERATEGroundAck == 1 && DataModel.KEYFRAMERATEAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                KEYFRAMERATE.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.KEYFRAMERATEGroundAck = 0;
                DataModel.KEYFRAMERATEAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                KEYFRAMERATE.setText(Data);
            }
        }
        if(Val.equals("FPS") == true)
        {
            EditText FPS = (EditText) findViewById(R.id.FPS);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FPSAirAck = 1;
                FPS.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FPSGroundAck = 1;
                FPS.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.FPSGroundAck == 1 && DataModel.FPSAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                FPS.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.FPSGroundAck = 0;
                DataModel.FPSAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                FPS.setText(Data);
            }
        }
        if(Val.equals("FREQSCAN") == true)
        {
            EditText FREQSCAN = (EditText) findViewById(R.id.FREQSCAN);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FREQSCANAirAck = 1;
                FREQSCAN.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FREQSCANGroundAck = 1;
                FREQSCAN.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.FREQSCANGroundAck == 1 && DataModel.FREQSCANAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                FREQSCAN.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.FREQSCANGroundAck = 0;
                DataModel.FREQSCANAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                FREQSCAN.setText(Data);
            }
        }
        if(Val.equals("TXMODE") == true)
        {
            EditText TXMODE = (EditText) findViewById(R.id.TXMODE);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.TXMODEAirAck = 1;
                TXMODE.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.TXMODEGroundAck = 1;
                TXMODE.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.TXMODEGroundAck == 1 && DataModel.TXMODEAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                TXMODE.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.TXMODEGroundAck = 0;
                DataModel.TXMODEAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                TXMODE.setText(Data);
            }
        }
        if(Val.equals("TELEMETRY_TRANSMISSION") == true)
        {
            EditText TELEMETRY_TRANSMISSION = (EditText) findViewById(R.id.TELEMETRY_TRANSMISSION);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.TELEMETRY_TRANSMISSIONAirAck = 1;
                TELEMETRY_TRANSMISSION.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.TELEMETRY_TRANSMISSIONGroundAck = 1;
                TELEMETRY_TRANSMISSION.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.TELEMETRY_TRANSMISSIONGroundAck == 1 && DataModel.TELEMETRY_TRANSMISSIONAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                TELEMETRY_TRANSMISSION.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.TELEMETRY_TRANSMISSIONGroundAck = 0;
                DataModel.TELEMETRY_TRANSMISSIONAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                TELEMETRY_TRANSMISSION.setText(Data);
            }
        }
        if(Val.equals("TELEMETRY_UPLINK") == true)
        {
            EditText TELEMETRY_UPLINK = (EditText) findViewById(R.id.TELEMETRY_UPLINK);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.TELEMETRY_UPLINKAirAck = 1;
                TELEMETRY_UPLINK.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.TELEMETRY_UPLINKGroundAck = 1;
                TELEMETRY_UPLINK.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.TELEMETRY_UPLINKGroundAck == 1 && DataModel.TELEMETRY_UPLINKAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                TELEMETRY_UPLINK.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.TELEMETRY_UPLINKGroundAck = 0;
                DataModel.TELEMETRY_UPLINKAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                TELEMETRY_UPLINK.setText(Data);
            }
        }
        if(Val.equals("RC") == true)
        {
            EditText RC = (EditText) findViewById(R.id.RC);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.RCAirAck = 1;
                RC.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.RCGroundAck = 1;
                RC.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.RCGroundAck == 1 && DataModel.RCAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                RC.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.RCGroundAck = 0;
                DataModel.RCAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                RC.setText(Data);
            }
        }
        if(Val.equals("CTS_PROTECTION") == true)
        {
            EditText CTS_PROTECTION = (EditText) findViewById(R.id.CTS_PROTECTION);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.CTS_PROTECTIONAirAck = 1;
                CTS_PROTECTION.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.CTS_PROTECTIONGroundAck = 1;
                CTS_PROTECTION.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.CTS_PROTECTIONGroundAck == 1 && DataModel.CTS_PROTECTIONAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                CTS_PROTECTION.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.CTS_PROTECTIONGroundAck = 0;
                DataModel.CTS_PROTECTIONAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                CTS_PROTECTION.setText(Data);
            }
        }
        if(Val.equals("WIFI_HOTSPOT") == true)
        {
            EditText WIFI_HOTSPOT = (EditText) findViewById(R.id.WIFI_HOTSPOT);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.WIFI_HOTSPOTAirAck = 1;
                WIFI_HOTSPOT.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.WIFI_HOTSPOTGroundAck = 1;
                WIFI_HOTSPOT.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.WIFI_HOTSPOTGroundAck == 1 && DataModel.WIFI_HOTSPOTAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                WIFI_HOTSPOT.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.WIFI_HOTSPOTGroundAck = 0;
                DataModel.WIFI_HOTSPOTAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                WIFI_HOTSPOT.setText(Data);
            }
        }
        if(Val.equals("WIFI_HOTSPOT_NIC") == true)
        {
            EditText WIFI_HOTSPOT_NIC = (EditText) findViewById(R.id.WIFI_HOTSPOT_NIC);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.WIFI_HOTSPOT_NICAirAck = 1;
                WIFI_HOTSPOT_NIC.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.WIFI_HOTSPOT_NICGroundAck = 1;
                WIFI_HOTSPOT_NIC.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.WIFI_HOTSPOT_NICGroundAck == 1 && DataModel.WIFI_HOTSPOT_NICAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                WIFI_HOTSPOT_NIC.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.WIFI_HOTSPOT_NICGroundAck = 0;
                DataModel.WIFI_HOTSPOT_NICAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                WIFI_HOTSPOT_NIC.setText(Data);
            }
        }
        if(Val.equals("ETHERNET_HOTSPOT") == true)
        {
            EditText ETHERNET_HOTSPOT = (EditText) findViewById(R.id.ETHERNET_HOTSPOT);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.ETHERNET_HOTSPOTAirAck = 1;
                ETHERNET_HOTSPOT.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.ETHERNET_HOTSPOTGroundAck = 1;
                ETHERNET_HOTSPOT.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.ETHERNET_HOTSPOTGroundAck == 1 && DataModel.ETHERNET_HOTSPOTAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                ETHERNET_HOTSPOT.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.ETHERNET_HOTSPOTGroundAck = 0;
                DataModel.ETHERNET_HOTSPOTAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                ETHERNET_HOTSPOT.setText(Data);
            }
        }
        if(Val.equals("ENABLE_SCREENSHOTS") == true)
        {
            EditText ENABLE_SCREENSHOTS = (EditText) findViewById(R.id.ENABLE_SCREENSHOTS);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.ENABLE_SCREENSHOTSAirAck = 1;
                ENABLE_SCREENSHOTS.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.ENABLE_SCREENSHOTSGroundAck = 1;
                ENABLE_SCREENSHOTS.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.ENABLE_SCREENSHOTSGroundAck == 1 && DataModel.ENABLE_SCREENSHOTSAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                ENABLE_SCREENSHOTS.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.ENABLE_SCREENSHOTSGroundAck = 0;
                DataModel.ENABLE_SCREENSHOTSAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                ENABLE_SCREENSHOTS.setText(Data);
            }
        }
        if(Val.equals("FORWARD_STREAM") == true)
        {
            EditText FORWARD_STREAM = (EditText) findViewById(R.id.FORWARD_STREAM);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FORWARD_STREAMAirAck = 1;
                FORWARD_STREAM.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.FORWARD_STREAMGroundAck = 1;
                FORWARD_STREAM.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.FORWARD_STREAMGroundAck == 1 && DataModel.FORWARD_STREAMAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                FORWARD_STREAM.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.FORWARD_STREAMGroundAck = 0;
                DataModel.FORWARD_STREAMAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                FORWARD_STREAM.setText(Data);
            }
        }
        if(Val.equals("VIDEO_UDP_PORT") == true)
        {
            EditText VIDEO_UDP_PORT = (EditText) findViewById(R.id.VIDEO_UDP_PORT);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.VIDEO_UDP_PORTAirAck = 1;
                VIDEO_UDP_PORT.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.VIDEO_UDP_PORTGroundAck = 1;
                VIDEO_UDP_PORT.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.VIDEO_UDP_PORTGroundAck == 1 && DataModel.VIDEO_UDP_PORTAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                VIDEO_UDP_PORT.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.VIDEO_UDP_PORTGroundAck = 0;
                DataModel.VIDEO_UDP_PORTAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                VIDEO_UDP_PORT.setText(Data);
            }
        }
        //Add Camera control
        if(Val.equals("IsCamera1Enabled") == true)
        {
            EditText IsCamera1Enabled = (EditText) findViewById(R.id.IsCamera1Enabled);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.IsCamera1EnabledAirAck = 1;
                IsCamera1Enabled.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.IsCamera1EnabledGroundAck = 1;
                IsCamera1Enabled.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.IsCamera1EnabledGroundAck == 1 && DataModel.IsCamera1EnabledAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                IsCamera1Enabled.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.IsCamera1EnabledGroundAck = 0;
                DataModel.IsCamera1EnabledAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                IsCamera1Enabled.setText(Data);
            }
        }

        if(Val.equals("IsCamera2Enabled") == true)
        {
            EditText IsCamera2Enabled = (EditText) findViewById(R.id.IsCamera2Enabled);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.IsCamera2EnabledAirAck = 1;
                IsCamera2Enabled.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.IsCamera2EnabledGroundAck = 1;
                IsCamera2Enabled.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.IsCamera2EnabledGroundAck == 1 && DataModel.IsCamera2EnabledAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                IsCamera2Enabled.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.IsCamera2EnabledGroundAck = 0;
                DataModel.IsCamera2EnabledAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                IsCamera2Enabled.setText(Data);
            }
        }

        if(Val.equals("IsCamera3Enabled") == true)
        {
            EditText IsCamera3Enabled = (EditText) findViewById(R.id.IsCamera3Enabled);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.IsCamera3EnabledAirAck = 1;
                IsCamera3Enabled.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.IsCamera3EnabledGroundAck = 1;
                IsCamera3Enabled.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.IsCamera3EnabledGroundAck == 1 && DataModel.IsCamera3EnabledAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                IsCamera3Enabled.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.IsCamera3EnabledGroundAck = 0;
                DataModel.IsCamera3EnabledAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                IsCamera3Enabled.setText(Data);
            }
        }

        if(Val.equals("IsCamera4Enabled") == true)
        {
            EditText IsCamera4Enabled = (EditText) findViewById(R.id.IsCamera4Enabled);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.IsCamera4EnabledAirAck = 1;
                IsCamera4Enabled.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.IsCamera4EnabledGroundAck = 1;
                IsCamera4Enabled.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.IsCamera4EnabledGroundAck == 1 && DataModel.IsCamera4EnabledAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                IsCamera4Enabled.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.IsCamera4EnabledGroundAck = 0;
                DataModel.IsCamera4EnabledAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                IsCamera4Enabled.setText(Data);
            }
        }

        if(Val.equals("DefaultCameraId") == true)
        {
            EditText DefaultCameraId = (EditText) findViewById(R.id.DefaultCameraId);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.DefaultCameraIdAirAck = 1;
                DefaultCameraId.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.DefaultCameraIdGroundAck = 1;
                DefaultCameraId.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.DefaultCameraIdGroundAck == 1 && DataModel.DefaultCameraIdAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                DefaultCameraId.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.DefaultCameraIdGroundAck = 0;
                DataModel.DefaultCameraIdAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                DefaultCameraId.setText(Data);
            }
        }

        if(Val.equals("ChannelToListen") == true)
        {
            EditText ChannelToListen = (EditText) findViewById(R.id.ChannelToListen);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.ChannelToListenAirAck = 1;
                ChannelToListen.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.ChannelToListenGroundAck = 1;
                ChannelToListen.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.ChannelToListenGroundAck == 1 && DataModel.ChannelToListenAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                ChannelToListen.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.ChannelToListenGroundAck = 0;
                DataModel.ChannelToListenAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                ChannelToListen.setText(Data);
            }
        }

        if(Val.equals("Camera1ValueMin") == true)
        {
            EditText Camera1ValueMin = (EditText) findViewById(R.id.Camera1ValueMin);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera1ValueMinAirAck = 1;
                Camera1ValueMin.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera1ValueMinGroundAck = 1;
                Camera1ValueMin.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.Camera1ValueMinGroundAck == 1 && DataModel.Camera1ValueMinAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Camera1ValueMin.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.Camera1ValueMinGroundAck = 0;
                DataModel.Camera1ValueMinAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Camera1ValueMin.setText(Data);
            }
        }

        if(Val.equals("Camera1ValueMax") == true)
        {
            EditText Camera1ValueMax = (EditText) findViewById(R.id.Camera1ValueMax);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera1ValueMaxAirAck = 1;
                Camera1ValueMax.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera1ValueMaxGroundAck = 1;
                Camera1ValueMax.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.Camera1ValueMaxGroundAck == 1 && DataModel.Camera1ValueMaxAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Camera1ValueMax.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.Camera1ValueMaxGroundAck = 0;
                DataModel.Camera1ValueMaxAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Camera1ValueMax.setText(Data);
            }
        }

        if(Val.equals("Camera2ValueMin") == true)
        {
            EditText Camera2ValueMin = (EditText) findViewById(R.id.Camera2ValueMin);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera2ValueMinAirAck = 1;
                Camera2ValueMin.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera2ValueMinGroundAck = 1;
                Camera2ValueMin.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.Camera2ValueMinGroundAck == 1 && DataModel.Camera2ValueMinAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Camera2ValueMin.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.Camera2ValueMinGroundAck = 0;
                DataModel.Camera2ValueMinAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Camera2ValueMin.setText(Data);
            }
        }

        if(Val.equals("Camera2ValueMax") == true)
        {
            EditText Camera2ValueMax = (EditText) findViewById(R.id.Camera2ValueMax);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera2ValueMaxAirAck = 1;
                Camera2ValueMax.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera2ValueMaxGroundAck = 1;
                Camera2ValueMax.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.Camera2ValueMaxGroundAck == 1 && DataModel.Camera2ValueMaxAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Camera2ValueMax.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.Camera2ValueMaxGroundAck = 0;
                DataModel.Camera2ValueMaxAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Camera2ValueMax.setText(Data);
            }

        }

        if(Val.equals("Camera3ValueMin") == true)
        {
            EditText Camera3ValueMin = (EditText) findViewById(R.id.Camera3ValueMin);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera3ValueMinAirAck = 1;
                Camera3ValueMin.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera3ValueMinGroundAck = 1;
                Camera3ValueMin.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.Camera3ValueMinGroundAck == 1 && DataModel.Camera3ValueMinAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Camera3ValueMin.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.Camera3ValueMinGroundAck = 0;
                DataModel.Camera3ValueMinAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Camera3ValueMin.setText(Data);
            }
        }

        if(Val.equals("Camera3ValueMax") == true)
        {
            EditText Camera3ValueMax = (EditText) findViewById(R.id.Camera3ValueMax);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera3ValueMaxAirAck = 1;
                Camera3ValueMax.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera3ValueMaxGroundAck = 1;
                Camera3ValueMax.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.Camera3ValueMaxGroundAck == 1 && DataModel.Camera3ValueMaxAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Camera3ValueMax.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.Camera3ValueMaxGroundAck = 0;
                DataModel.Camera3ValueMaxAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Camera3ValueMax.setText(Data);
            }
        }

        if(Val.equals("Camera4ValueMin") == true)
        {
            EditText Camera4ValueMin = (EditText) findViewById(R.id.Camera4ValueMin);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera4ValueMinAirAck = 1;
                Camera4ValueMin.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera4ValueMinGroundAck = 1;
                Camera4ValueMin.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.Camera4ValueMinGroundAck == 1 && DataModel.Camera4ValueMinAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Camera4ValueMin.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.Camera4ValueMinGroundAck = 0;
                DataModel.Camera4ValueMinAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Camera4ValueMin.setText(Data);
            }
        }

        if(Val.equals("Camera4ValueMax") == true)
        {
            EditText Camera4ValueMax = (EditText) findViewById(R.id.Camera4ValueMax);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera4ValueMaxAirAck = 1;
                Camera4ValueMax.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Camera4ValueMaxGroundAck = 1;
                Camera4ValueMax.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.Camera4ValueMaxGroundAck == 1 && DataModel.Camera4ValueMaxAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Camera4ValueMax.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.Camera4ValueMaxGroundAck = 0;
                DataModel.Camera4ValueMaxAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Camera4ValueMax.setText(Data);
            }
        }

        if(Val.equals("CELLS") == true)
        {
            EditText CELLS = (EditText) findViewById(R.id.CELLS);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.CELLSAirAck = 1;
                CELLS.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.CELLSGroundAck = 1;
                CELLS.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.CELLSGroundAck == 1 && DataModel.CELLSAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                CELLS.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.CELLSGroundAck = 0;
                DataModel.CELLSAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                CELLS.setText(Data);
            }
        }

        if(Val.equals("EncryptionOrRange") == true)
        {
            //EditText FC_RC_BAUDRATE = (EditText) findViewById(R.id.EditFC_RC_BAUDRATE);
            //FC_RC_BAUDRATE.setText(Data);
            //mySpinner.setSelection(adapter.getPosition(Data));
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.EncryptionOrRangeAirAck = 1;
                SpinnerEncryptionOrRange.setBackgroundColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.EncryptionOrRangeGroundAck = 1;
                SpinnerEncryptionOrRange.setBackgroundColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.EncryptionOrRangeGroundAck == 1 && DataModel.EncryptionOrRangeAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                SpinnerEncryptionOrRange.setBackgroundColor(Color.parseColor(ColorsBoth) );
                DataModel.EncryptionOrRangeGroundAck = 0;
                DataModel.EncryptionOrRangeAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                String selectedNow = SpinnerEncryptionOrRange.getSelectedItem().toString();
                if(selectedNow.equals(Data) == true)
                    DataModel.EncryptionOrRangeIsChanged = -1;
                if(selectedNow.equals("value not loaded") == true)
                    DataModel.EncryptionOrRangeIsChanged = -1;
                SpinnerEncryptionOrRange.setSelection(((ArrayAdapter)SpinnerEncryptionOrRange.getAdapter()).getPosition(Data));
            }
        }

        if(Val.equals("IsBandSwicherEnabled") == true)
        {
            EditText IsBandSwicherEnabled = (EditText) findViewById(R.id.IsBandSwicherEnabled);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.IsBandSwicherEnabledAirAck = 1;
                IsBandSwicherEnabled.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.IsBandSwicherEnabledGroundAck = 1;
                IsBandSwicherEnabled.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.IsBandSwicherEnabledGroundAck == 1 && DataModel.IsBandSwicherEnabledAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                IsBandSwicherEnabled.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.IsBandSwicherEnabledGroundAck = 0;
                DataModel.IsBandSwicherEnabledAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                IsBandSwicherEnabled.setText(Data);
            }
        }

        if(Val.equals("Bandwidth") == true)
        {
            EditText Bandwidth = (EditText) findViewById(R.id.Bandwidth);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.BandwidthAirAck = 1;
                Bandwidth.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.BandwidthGroundAck = 1;
                Bandwidth.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.BandwidthGroundAck == 1 && DataModel.BandwidthAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Bandwidth.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.BandwidthGroundAck = 0;
                DataModel.BandwidthAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Bandwidth.setText(Data);
            }
        }

        if(Val.equals("UplinkSpeed") == true)
        {
            EditText UplinkSpeed = (EditText) findViewById(R.id.UplinkSpeed);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.UplinkSpeedAirAck = 1;
                UplinkSpeed.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.UplinkSpeedGroundAck = 1;
                UplinkSpeed.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.UplinkSpeedGroundAck == 1 && DataModel.UplinkSpeedAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                UplinkSpeed.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.UplinkSpeedGroundAck = 0;
                DataModel.UplinkSpeedAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                UplinkSpeed.setText(Data);
            }
        }

        if(Val.equals("ChannelToListen2") == true)
        {
            EditText ChannelToListen2 = (EditText) findViewById(R.id.ChannelToListen2);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.ChannelToListen2AirAck = 1;
                ChannelToListen2.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.ChannelToListen2GroundAck = 1;
                ChannelToListen2.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.ChannelToListen2GroundAck == 1 && DataModel.ChannelToListen2AirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                ChannelToListen2.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.ChannelToListen2GroundAck = 0;
                DataModel.ChannelToListen2AirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                ChannelToListen2.setText(Data);
            }
        }

        if(Val.equals("PrimaryCardMAC") == true)
        {
            EditText PrimaryCardMAC = (EditText) findViewById(R.id.PrimaryCardMAC);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.PrimaryCardMACAirAck = 1;
                PrimaryCardMAC.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.PrimaryCardMACGroundAck = 1;
                PrimaryCardMAC.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.PrimaryCardMACGroundAck == 1 && DataModel.PrimaryCardMACAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                PrimaryCardMAC.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.PrimaryCardMACGroundAck = 0;
                DataModel.PrimaryCardMACAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                PrimaryCardMAC.setText(Data);
            }
        }

        if(Val.equals("SlaveCardMAC") == true)
        {
            EditText SlaveCardMAC = (EditText) findViewById(R.id.SlaveCardMAC);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.SlaveCardMACAirAck = 1;
                SlaveCardMAC.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.SlaveCardMACGroundAck = 1;
                SlaveCardMAC.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.SlaveCardMACGroundAck == 1 && DataModel.SlaveCardMACAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                SlaveCardMAC.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.SlaveCardMACGroundAck = 0;
                DataModel.SlaveCardMACAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                SlaveCardMAC.setText(Data);
            }
        }

        if(Val.equals("Band5Below") == true)
        {
            EditText Band5Below = (EditText) findViewById(R.id.Band5Below);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Band5BelowAirAck = 1;
                Band5Below.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Band5BelowGroundAck = 1;
                Band5Below.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.Band5BelowGroundAck == 1 && DataModel.Band5BelowAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Band5Below.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.Band5BelowGroundAck = 0;
                DataModel.Band5BelowAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Band5Below.setText(Data);
            }
        }

        if(Val.equals("Band10ValueMin") == true)
        {
            EditText Band10ValueMin = (EditText) findViewById(R.id.Band10ValueMin);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Band10ValueMinAirAck = 1;
                Band10ValueMin.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Band10ValueMinGroundAck = 1;
                Band10ValueMin.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.Band10ValueMinGroundAck == 1 && DataModel.Band10ValueMinAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Band10ValueMin.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.Band10ValueMinGroundAck = 0;
                DataModel.Band10ValueMinAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Band10ValueMin.setText(Data);
            }
        }

        if(Val.equals("Band10ValueMax") == true)
        {
            EditText Band10ValueMax = (EditText) findViewById(R.id.Band10ValueMax);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Band10ValueMaxAirAck = 1;
                Band10ValueMax.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Band10ValueMaxGroundAck = 1;
                Band10ValueMax.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.Band10ValueMaxGroundAck == 1 && DataModel.Band10ValueMaxAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Band10ValueMax.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.Band10ValueMaxGroundAck = 0;
                DataModel.Band10ValueMaxAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Band10ValueMax.setText(Data);
            }
        }

        if(Val.equals("Band20After") == true)
        {
            EditText Band20After = (EditText) findViewById(R.id.Band20After);
            if(Data.equals("SavedAir") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedAir in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Band20AfterAirAck = 1;
                Band20After.setTextColor(Color.parseColor(ColorsAirOnly) );
            }
            if(Data.equals("SavedGround") == true)
            {
                //Toast.makeText(getApplicationContext(), "SavedGround in" + Data, Toast.LENGTH_LONG).show();
                DataModel.Band20AfterGroundAck = 1;
                Band20After.setTextColor(Color.parseColor(ColorsGroundOnly) );
            }
            if(DataModel.Band20AfterGroundAck == 1 && DataModel.Band20AfterAirAck == 1)
            {
                Toast.makeText(getApplicationContext(), Val + " \nAir and Ground are in sync", Toast.LENGTH_LONG).show();
                Band20After.setTextColor(Color.parseColor(ColorsBoth) );
                DataModel.Band20AfterGroundAck = 0;
                DataModel.Band20AfterAirAck = 0;
            }
            if(Data.equals("SavedGround") == false && Data.equals("SavedAir") == false)
            {
                //Toast.makeText(getApplicationContext(), "false and false in "+ Data, Toast.LENGTH_LONG).show();
                Band20After.setText(Data);
            }
        }

    }


    @Override
    public void onClick(View v) {

        switch (v.getId()) {

            case R.id.RequestSettings:
                RequestSettingsAll();
                break;
            case R.id.RequestSaveSettings:
                RequestSaveSettings();
                break;
        }
    }


}

