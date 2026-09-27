
#!/bin/sh

#
#set camera i2c pin mux
#

I2C_DEV=10;
I2C_ADDR=0x3b;

print_usage()
{
	echo "Usage:  ./veye_mipi_i2c.sh [-r/w] [-f] function name -p1 param1 -p2 param2 -b bus"
	echo "options:"
	echo "    -r                       read "
	echo "    -w                       write"
	echo "    -f [function name]       function name"
	echo "    -p1 [param1] 			   param1 of each function"
	echo "    -p2 [param1] 			   param2 of each function"
	echo "    -b [i2c bus num] 		   i2c bus number"
	echo "    -d [i2c addr] 		   i2c addr if not default 0x3b"
	echo "support functions: devid,hdver,sensorid,wdrmode,videoformat,mirrormode,denoise,agc,lowlight,daynightmode,ircutdir,irtrigger,mshutter,"
    echo "cameramode, nodf, capture, csienable,saturation,wdrbtargetbr,wdrtargetbr, brightness ,contrast , sharppen,wdrsharppen aespeed,lsc,boardmodel,yuvseq,i2cauxenable,i2cwen,awbgain,wbmode,mwbgain,antiflicker,awb_boffset,blcstrength,blcpos,exptime,paramsave"
    echo "new_expmode,new_mshutter,new_mgain"
}

######################parse arg###################################
MODE=read;
FUNCTION=version;
SENSOR_ID=0;
VIDEO_FORMAT=1;# 1 ntsc,0 pal
PARAM1=0;
PARAM2=0;
PARAM3=0;
PARAM4=0;
b_arg_param1=0;
b_arg_param2=0;
b_arg_param3=0;
b_arg_param4=0;
b_arg_functin=0;
b_arg_bus=0;
b_arg_addr=0;

for arg in $@
do
	if [ $b_arg_functin -eq 1 ]; then
		b_arg_functin=0;
		FUNCTION=$arg;
		if [ -z $FUNCTION ]; then
			echo "[error] FUNCTION is null"
			exit;
		fi
	fi
	if [ $b_arg_param1 -eq 1 ] ; then
		b_arg_param1=0;
		PARAM1=$arg;
	fi

	if [ $b_arg_param2 -eq 1 ] ; then
		b_arg_param2=0;
		PARAM2=$arg;
	fi
    if [ $b_arg_param3 -eq 1 ] ; then
		b_arg_param3=0;
		PARAM3=$arg;
	fi
    if [ $b_arg_param4 -eq 1 ] ; then
		b_arg_param4=0;
		PARAM4=$arg;
	fi
	if [ $b_arg_bus -eq 1 ] ; then
		b_arg_bus=0;
		I2C_DEV=$arg;
	fi
    if [ $b_arg_addr -eq 1 ] ; then
		b_arg_addr=0;
		I2C_ADDR=$arg;
	fi
	case $arg in
		"-r")
			MODE=read;
			;;
		"-w")
			MODE=write;
			;;
		"-f")			
			b_arg_functin=1;
			;;
		"-p1")
			b_arg_param1=1;
			;;
		"-p2")
			b_arg_param2=1;
			;;
        "-p3")
			b_arg_param3=1;
			;;
        "-p4")
			b_arg_param4=1;
			;;
		"-b")
			b_arg_bus=1;
			;;
        "-d")
			b_arg_addr=1;
			;;
		"-h")
			print_usage;
			;;
	esac
done
#######################parse arg end########################

#######################Action###############################
if [ $# -lt 1 ]; then
    print_usage;
    exit 0;
fi

pinmux()
{
	sh ./camera_i2c_config >> /dev/null 2>&1
}

read_devid()
{
	local verid=0;
	local res=0;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x01 );
	verid=$?;
	printf "device id is 0x%2x\n" $verid;
}

read_releasedate()
{
    local yy=0;
    local mm=0;
    local dd=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x5C );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	yy=$?;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x5D );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	mm=$?;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x52 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	dd=$?;
	printf "release date is 20%2x-%2x-%2x\n" $yy $mm $dd;
}

read_hardver()
{
	local hardver=0;
	local res=0;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x00 );
	hardver=$?;
	printf "hardware version is 0x%2x\n" $hardver;
    	read_releasedate;
}
#define SENSOR_TYPR_ADDR_L    0x20
#define SENSOR_TYPR_ADDR_H    0x21
#define BOARD_TYPR_ADDR    0x25
__read_sensorid()
{
    local sensorid_l=0;
    local sensorid_h=0;
    local res=0;
    res=$(i2c_read $I2C_DEV $I2C_ADDR 0x20);
	sensorid_h=$?;
    res=$(i2c_read $I2C_DEV $I2C_ADDR 0x21);
	sensorid_l=$?;
    SENSOR_ID=$((($sensorid_h<<8)+$sensorid_l));
    #printf "read sensor id %x" $SENSOR_ID;
}
read_sensorid()
{
    local board_type=0;
    local res=0;
    __read_sensorid;
    printf "r sensor id is IMX%x;" $SENSOR_ID;
    
    res=$(i2c_read $I2C_DEV $I2C_ADDR 0x25);
	board_type=$?;
    if [ $board_type -eq 76 ] ; then
		printf " ONE board\n";
	else
        printf " TWO board\n";
    fi
}

read_wdrmode()
{
	local wdrmode=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDB );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x32 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	wdrmode=$?;
	printf "r wdrmode is 0x%2x\n" $wdrmode;
}
write_wdrmode()
{
	local wdrmode=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDB );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x32 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w wdrmode is 0x%2x\n" $PARAM1;
}

__read_videoformat()
{
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDE );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0xC2 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	VIDEO_FORMAT=$?;
    #printf "r Video Format is %d\n" $VIDEO_FORMAT;
}

read_videoformat()
{
	local res=0;
    __read_videoformat;
	if [ $VIDEO_FORMAT -eq 1 ] ; then
		printf "r Video Format is NTSC(60Hz)\n";
	elif [ $VIDEO_FORMAT -eq 0 ] ; then
		printf "r Video Format is PAL(50Hz)\n";
	fi
}

write_videoformat()
{
	local videoformat=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDE );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0xC2 );
	if [ $PARAM1 = "PAL" ] ; then
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 0x0);
	elif [ $PARAM1 = "NTSC" ] ; then
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 0x1);
	fi
	
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w videoformat is %s\n" $PARAM1;
}

read_mirrormode()
{
	local mirrormode=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDE );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x57 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	mirrormode=$?;
	printf "r mirrormode is 0x%2x\n" $mirrormode;
}

write_mirrormode()
{
	local mirrormode=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDE );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x57 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w mirrormode is 0x%2x\n" $PARAM1;
}

read_denoise()
{
	local denoise=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD8 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x9B );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	denoise=$?;
	printf "r denoise is 0x%2x\n" $denoise;
}
write_denoise()
{
	local denoise=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD8 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x9B );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w denoise is 0x%2x\n" $PARAM1;
}

read_agc()
{
	local agc=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x67 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
    agc=$(($?&0x0F));
	printf "r agc is 0x%02x\n" $agc;
}
write_agc()
{
	local agc=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x67 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
    agc=$(($?&0xF0));
    printf "w agc is 0x%2x\n" $PARAM1;
    PARAM1=$(($agc|$PARAM1));
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x67 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	
}

read_lowlight()
{
	local lowlight=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x64 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	lowlight=$?;
	printf "r lowlight is 0x%2x\n" $lowlight;
}
write_lowlight()
{
	local lowlight=0;
	local res=0;
	if [ $(printf "%d" $PARAM1) -eq 0 ]; then
		printf "close lowlight mode\n";
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x6D );
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 0xA5);
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
		sleep 0.01;
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x66 );
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 0x40);
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	else
		printf "open lowlight mode\n";
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x6D );
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 0xA4);
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
		sleep 0.01;
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x66 );
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 0x41 );
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	fi
	sleep 0.01;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x64 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w lowlight is 0x%2x\n" $PARAM1;
}

read_mshutter()
{
	local mshutter=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x66 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	mshutter=$?;
	printf "r mshutter is 0x%2x\n" $mshutter;
}
write_mshutter()
{
	local mshutter=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x66 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w mshutter is 0x%2x\n" $PARAM1;
}

read_daynightmode()
{
	local daynightmode=0;
	local res=0;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x02 );
	daynightmode=$?;
	printf "r daynightmode is 0x%2x\n" $daynightmode;
}
write_daynightmode()
{
	local daynightmode=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x02 $PARAM1);
	printf "w daynightmode is 0x%2x\n" $PARAM1;
}
read_ircutdir()
{
	local ircutdir=0;
	local res=0;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x16 );
	ircutdir=$?;
	printf "r ircutdir is 0x%2x\n" $ircutdir;
}
write_ircutdir()
{
	local ircutdir=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x16 $PARAM1);
	printf "w ircutdir is 0x%2x\n" $PARAM1;
}

read_irtrigger()
{
	local irtrigger=0;
	local res=0;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x15 );
	irtrigger=$?;
	printf "r irtrigger is 0x%2x\n" $irtrigger;
}
write_irtrigger()
{
	local irtrigger=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR 0x15 $PARAM1);
	printf "w irtrigger is 0x%2x\n" $PARAM1;
}

read_cameramode()
{
	local cameramode=0;
	local res=0;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x1A );
	cameramode=$?;
	printf "r cameramode is 0x%2x\n" $cameramode;
}
write_cameramode()
{
	local cameramode=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR 0x1A $PARAM1);
	printf "w cameramode is 0x%2x\n" $PARAM1;
}

read_nodf()
{
	local nodf=0;
	local res=0;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x1B );
	nodf=$?;
	printf "r nodf is 0x%2x\n" $nodf;
}

write_nodf()
{
	local nodf=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR 0x1B $PARAM1);
	printf "w nodf is 0x%2x\n" $PARAM1;
}

write_capture()
{
	local capture=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR 0x1C 0x1);
	printf "w capture\n" ;
}

read_csienable()
{
	local csienable=0;
	local res=0;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x1D );
	csienable=$?;
	printf "r csienable is 0x%2x\n" $csienable;
}

write_csienable()
{
	local csienable=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR 0x1D $PARAM1);
	printf "w csienable is 0x%2x\n" $PARAM1;
}

read_yuvseq()
{
	local yuvseq=0;
	local res=0;
	res=$(i2c_read $I2C_DEV $I2C_ADDR 0x1E );
	yuvseq=$?;
    if [ $yuvseq -eq 1 ] ; then
		printf "r YUVseq is YUYV\n";
    else
        printf "r YUVseq is UYVY\n";
	fi
}

write_yuvseq()
{
	local csienable=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR 0x1E $PARAM1);
    if [ $PARAM1 = "YUYV" ] ; then
		res=$(i2c_write $I2C_DEV $I2C_ADDR  0x1E 0x1);
    else
        res=$(i2c_write $I2C_DEV $I2C_ADDR  0x1E 0x0);
	fi
	printf "w YUVseq is %s\n" $PARAM1;
}

read_board_model()
{
    local board_model=0;
	local res=0;
	res=$(i2c_read $I2C_DEV $I2C_ADDR 0x25);
	board_model=$?;
     if [ $board_model -eq 255 ] ; then
		printf "Board model is VEYE-MIPI-327 \n";
	elif [ $board_model -eq 76 ] ; then
		printf "Board model is VEYE-MIPI-IMX327S\n";
	fi
}

read_i2c_aux_enable()
{
    local i2c_aux_enable=0;
	local res=0;
	res=$(i2c_read $I2C_DEV $I2C_ADDR 0x1F);
	i2c_aux_enable=$?;
	if [ $i2c_aux_enable -eq 238 ] ; then
		printf "i2c aux is enable \n";
	else
		printf "i2c aux is disable\n";
	fi
}

write_i2c_aux_enable()
{
    local i2c_aux_enable=0;
	local res=0;
    if [ $PARAM1 -eq 0 ] ; then
        i2c_aux_enable=0x11;
    else
		i2c_aux_enable=0xEE;
	fi
	res=$(i2c_write $I2C_DEV $I2C_ADDR 0x1F $i2c_aux_enable);
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR 0x1F $i2c_aux_enable);
	printf "w i2c_aux_enable is 0x%2x\n" $PARAM1;
}

read_i2c_write_enable()
{
    local i2cwenable=0;
	local res=0;
	res=$(i2c_read $I2C_DEV $I2C_ADDR 0x24);
	i2cwenable=$?;
    if [ $i2cwenable -eq 238 ] ; then
        printf "r i2c write enable\n";
    else
		printf "r i2c write disable\n";
	fi
	
}

write_i2c_write_enable()
{
    local i2c_write_enable=0;
	local res=0;
    if [ $PARAM1 -eq 0 ] ; then
        i2c_write_enable=0x11;
    else
		i2c_write_enable=0xEE;
	fi

	res=$(i2c_write $I2C_DEV $I2C_ADDR 0x24 $i2c_write_enable);
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR 0x24 $i2c_write_enable);
	printf "w i2c_write_enable is 0x%2x\n" $PARAM1;
}

read_saturation()
{
	local saturation=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD8 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x7A );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	saturation=$?;
	printf "r saturation is 0x%2x\n" $saturation;
}
write_saturation()
{
	local saturation=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD8 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x7A );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD8 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x7B );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w saturation is 0x%2x\n" $PARAM1;
}

read_wdrbtargetbr()
{
	local wdrbtargetbr=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0xCA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	wdrbtargetbr=$?;
	printf "r wdrbtargetbr is 0x%2x\n" $wdrbtargetbr;
}
write_wdrbtargetbr()
{
	local wdrbtargetbr=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0xCA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w wdrbtargetbr is 0x%2x\n" $PARAM1;
}
    
read_brightness()
{
    local videoformat=0;
    local brightness=0;
	local res=0;
    __read_videoformat;
    if [ $VIDEO_FORMAT -eq 1 ] ; then
        res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
        res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x65 );
		printf "Video Format is NTSC(60Hz) \n";
	fi
	if [ $VIDEO_FORMAT -eq 0 ] ; then
        res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
        res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x1A );
		printf "Video Format is PAL(50Hz)\n";
	fi
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	brightness=$?;
	printf "r brightness is 0x%2x\n" $brightness;
}
write_brightness()
{
    local videoformat=0;
	local res=0;
    __read_videoformat;
    if [ $VIDEO_FORMAT -eq 1 ] ; then
        res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
        res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x65 );
		printf "Video Format is NTSC(60Hz) \n";
	fi
	if [ $VIDEO_FORMAT -eq 0 ] ; then
        res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
        res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x1A );
		printf "Video Format is PAL(50Hz)\n";
	fi
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1 );
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w brightness is 0x%2x\n" $PARAM1;
}


read_sharppen()
{
    local sharppen_enable=0;
    local sharppen_val=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD9 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x5D );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	sharppen_enable=$?;
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD9 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x52 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	sharppen_val=$(($?>>4));
	printf "r sharppen enable is 0x%2x val is 0x%2x\n" $sharppen_enable $sharppen_val;
}

write_sharppen()
{
    local sharppen_enable=0;
    local sharppen_val=0;
	local res=0;

	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD9 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x5D );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    
    sharppen_val=$(($PARAM2<<4|0x3));
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD9 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x52 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $sharppen_val);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w sharppen enable is 0x%2x val is 0x%2x \n" $PARAM1 $PARAM2;
}

read_wdrsharppen()
{
    local sharppen_enable=0;
    local sharppen_val=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDF );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0xF2 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	sharppen_enable=$(( ($? >> 2) & 1))
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDF );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0xF3 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	sharppen_val=$(($?>>4));
	printf "r WDR sharppen enable is 0x%2x val is 0x%2x\n" $sharppen_enable $sharppen_val;
}

write_wdrsharppen()
{
    local sharppen_enable=0;
    local sharppen_val=0;
	local res=0;
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDF );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0xF2 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	sharppen_enable=$(( ($PARAM1 << 2) | $?))
    
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDF );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0xF2 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $sharppen_enable);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    sleep 0.01;
    
    sharppen_val=$(($PARAM2<<4|0x0F));
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDF );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0xF3 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $sharppen_val);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w WDR sharppen enable is 0x%2x val is 0x%2x \n" $PARAM1 $PARAM2;
}
read_aespeed()
{
    local agcspeed=0;
    local shutterspeed=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x18 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	agcspeed=$?;
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x1B );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	shutterspeed=$?;
	printf "r agcspeed is 0x%2x shutter speed is 0x%2x\n" $agcspeed $shutterspeed;
}
write_aespeed()
{
    local agcspeed=0;
    local shutterspeed=0;
	local res=0;

	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x18 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x1B );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM2);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w agcspeed is 0x%2x shutterspeed is 0x%2x\n" $PARAM1 $PARAM2;
}

read_wdrtargetbr()
{
	local wdrtargetbr=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0xC1 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	wdrtargetbr=$?;
	printf "r wdrtargetbr is 0x%2x\n" $wdrtargetbr;
}
write_wdrtargetbr()
{
	local wdrtargetbr=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0xC1 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w wdrtargetbr is 0x%2x\n" $PARAM1;
}

read_contrast()
{
	local contrast=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0x49 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x5B );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    	sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	contrast=$?;
	printf "r contrast is 0x%2x\n" $contrast;
}
write_contrast()
{
	local contrast=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0x49 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x5B );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w contrast is 0x%2x\n" $PARAM1;
}

read_lsc_slop()
{
    local slop=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0x59 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x93 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x1 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
    slop=$?;
    printf "r LSC slop is %02x\n" $slop;
}
write_lsc_slop()
{
    local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0x59 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x93 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0x59 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x90 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    printf "w LSC slop %02x\n" $PARAM1;
}

read_lsc()
{
	local enable=0;
    local strength=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x77 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x1 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	enable=$?;
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x6A );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x1 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	strength=$?;
	printf "r LSC enable is 0x%2x and strength %02x \n" $enable $strength $slop;
}
write_lsc()
{
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x77 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x6A );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM2);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x6B );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM2);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x68 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM2);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );

    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x69 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM2);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x6E );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM2);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x6F );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM2);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    printf "w LSC enable is 0x%2x and strength %02x \n" $PARAM1 $PARAM2;
}
read_awbgain()
{
	local awbrgain=0;
    local awbbgain=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0x5E );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x0B );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	awbrgain=$?;
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0x5E );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x0F );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	awbbgain=$?;
    
	printf "r awb Rgain is 0x%2x Bgain is 0x%2x\n" $awbrgain $awbbgain ;
}

read_wbmode()
{
	local wbmode=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x34 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	wbmode=$?;
	printf "r awb mode is 0x%2x \n" $wbmode ;
}

write_wbmode()
{
    local wbmode=0;
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x34 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w wbmode is 0x%2x\n" $PARAM1;
}

read_mwbgain()
{
	local mwbrgain=0;
    local mwbbgain=0;
	local res=0;
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x2E );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	mwbrgain=$?;
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x29 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	mwbbgain=$?;
    
	printf "r mwb Rgain is 0x%2x Bgain is 0x%2x \n" $mwbrgain $mwbbgain ;
}

write_mwbgain()
{
	local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x2E );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x29 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM2);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    
	printf "w mwb Rgain is 0x%2x Bgain is 0x%2x \n" $PARAM1 $PARAM2;
}

read_antiflicker()
{
    local antiflicker=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x1F);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	antiflicker=$(($?>>6));
	printf "r antiflicker mode %x \n" $antiflicker ;
}

write_antiflicker()
{
    local res=0;
    local antiflicker=0;
    if [ $PARAM1 -eq 1 ] ; then
		antiflicker=0x40;
    else
        antiflicker=0x0;
	fi
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x1F );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $antiflicker);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    printf "w antiflicker %x\n" $PARAM1;
}
read_defog()
{
    local defog=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD9);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x2F);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	defog=$?;
	printf "r defog enable %x \n" $defog ;
}

write_defog()
{
    local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD9 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x2F );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    printf "w defog %x\n" $PARAM1;
}

read_blcstrength()
{
    local regval=0;
    local blcstrength=0;
    local blcgrad=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDB);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x33);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	regval=$?;
    blcstrength=$(($regval&0xF));
    blcgrad=$((($regval>>4)&0xF));
	printf "r blcstrength %d grad enable %d \n" $blcstrength $blcgrad;
}

write_blcstrength()
{
    local res=0;
    local blcstrength=$PARAM1;
    local blcgrad=$PARAM2;
    local regval=$((($blcgrad<<4)+$blcstrength));
    
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDB );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x33 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $regval);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    printf "w blcstrength %d grad enable %d regval %x\n" $blcstrength $blcgrad $regval;
}

read_blcpos()
{
    local start_xy=0;
    local size_xy=0;
    local start_x=0;
    local start_y=0;
    local size_x=0;
    local size_y=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDB);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x30);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	start_xy=$?;
    start_x=$(($start_xy&0xF));
    start_y=$((($start_xy>>4)&0xF));
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDB);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x31);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	size_xy=$?;
    size_x=$(($size_xy&0xF));
    size_y=$((($size_xy>>4)&0xF));
	printf "read BLC start x %d y %d sizex %d size y %d\n" $start_x $start_y $size_x $size_y ;
}

write_blcpos()
{
    local start_xy=0;
    local size_xy=0;
    local start_x=$PARAM1;
    local start_y=$PARAM2;
    local size_x=$PARAM3;
    local size_y=$PARAM4;
	local res=0;
    start_xy=$((($start_y<<4)+$start_x));
    size_xy=$((($size_y<<4)+$size_x));
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDB );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x30 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $start_xy);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDB );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x31 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $size_xy);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    printf "write BLC start x %d y %d sizex %d size y %d\n" $start_x $start_y $size_x $size_y ;
}

write_ispparamsave()
{
    local res=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x53 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 0x01);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    printf "w isp param save\n";
}

read_new_expmode()
{
    local regval=0;
    local expmode=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x67);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	regval=$?;
    expmode=$(($regval>>7));
    if [ $expmode -eq 1 ] ; then
		printf "r new expmode is manual, use new_mshutter and new_mgain\n";
    else
        printf "r new expmode is auto, use old mshutter and auto gain\n";
	fi
}

write_new_expmode()
{
    local regval=0;
    local expmode=$PARAM1;
    local res=0;
    if [ $expmode -eq 0 ] ; then
        regval=0x0C;
		printf "w new expmode is auto,  will use old mshutter and auto gain\n";
    else
        regval=0x8C;
        printf "w new expmode is manual, will use new_mshutter and new_mgain\n";
	fi
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x67 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $regval);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
}

read_new_mshutter()
{
    local regval=0;
    local time_1h=0;
    local mshutter=0;
    local res=0;
    local reg_l=0;
    local reg_h=0;
    local reg_val=0;
    local res=0;
    __read_videoformat;
    
    if [ $VIDEO_FORMAT -eq 1 ] ; then
		printf "r Video Format is NTSC(60Hz)\n";
        time_1h=14.815;#us
	elif [ $VIDEO_FORMAT -eq 0 ] ; then
		printf "r Video Format is PAL(50Hz)\n";
        time_1h=17.778;#us
	fi
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x1C);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	reg_h=$?;
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x1D);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	reg_l=$?;
    
    reg_val=$((($reg_h<<8)+$reg_l));
    mshutter=`echo $reg_val $time_1h | awk '{printf "%d\n",$1*$2}'`
    printf "r new mshutter is %d\n" $mshutter;
}

write_new_mshutter()
{
    local regval=0;
    local time_1h=0;
    local mshutter=0;
    local res=0;
    local reg_l=0;
    local reg_h=0;
    local reg_val=0;
    local res=0;
    __read_videoformat;
    
    if [ $VIDEO_FORMAT -eq 1 ] ; then
		printf "r Video Format is NTSC(60Hz)\n";
        time_1h=14.815;#us
	elif [ $VIDEO_FORMAT -eq 0 ] ; then
		printf "r Video Format is PAL(50Hz)\n";
        time_1h=17.778;#us
	fi
    
    mshutter=$PARAM1;
    #reg_val=$(($mshutter/$time_1h));
    reg_val=`echo $mshutter $time_1h | awk '{printf "%d\n",$1/$2}'`
    if [ $reg_val -gt 65535 ] ; then
        reg_val=65535;
        printf "mshutter time too long,will cut it!\n";
    fi
    reg_h=$(($reg_val>>8));
    reg_l=$(($reg_val&0xFF));
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x1C );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $reg_h);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x1D );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $reg_l);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    printf "w new_mshutter %d us\n" $mshutter;
}

read_new_mgain()
{
    local gainstep=0;
    local res=0;
    local reg_l=0;
    local reg_h=0;
    local reg_val=0;
    local mgain=0;
    
    __read_sensorid;
    if [ $SENSOR_ID -eq 901 ] ; then
        gainstep=0.1;
		printf "IMX385,gain step 0.1dB\n";
    else
        gainstep=0.3;
        printf "gain step 0.3dB\n";
	fi
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x06);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	reg_h=$?;
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x1E);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	reg_l=$?;
    
    reg_val=$((($reg_h<<8)+$reg_l));
    #mgain=$(($reg_val*$gainstep));
    mgain=`echo $reg_val $gainstep | awk '{printf "%.1f\n",$1*$2}'`
    printf "r new mgain %.1f dB\n" $mgain;
}

write_new_mgain()
{
    local gainstep=0;
    local res=0;
    local reg_l=0;
    local reg_h=0;
    local reg_val=0;
    local mgain=0;
    
    __read_sensorid;
    if [ $SENSOR_ID -eq 901 ] ; then
        gainstep=0.1;
		printf "IMX385,gain step 0.1dB\n";
    else
        gainstep=0.3;
        printf "gain step 0.3dB\n";
	fi
    mgain=$PARAM1;
    #reg_val=$(($mgain/$gainstep));
    reg_val=`echo $mgain $gainstep | awk '{printf "%d\n",$1/$2}'`
    reg_h=$(($reg_val>>8));
    reg_l=$(($reg_val&0xFF));
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x06 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $reg_h);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x1E );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $reg_l);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    
    printf "w new mgain %.1f dB\n" $mgain;
}

read_auto_shutter_max()
{
    local regval=0;
    local time_1h=0;
    local max_shutter=0;
    local res=0;
    local reg_l=0;
    local reg_h=0;
    local reg_val=0;
    local res=0;
    __read_videoformat;
    
    if [ $VIDEO_FORMAT -eq 1 ] ; then
		printf "r Video Format is NTSC(60Hz)\n";
        time_1h=14.815;#us
	elif [ $VIDEO_FORMAT -eq 0 ] ; then
		printf "r Video Format is PAL(50Hz)\n";
        time_1h=17.778;#us
	fi
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x04);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	reg_h=$?;
    
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x07);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	reg_l=$?;
    
    reg_val=$((($reg_h<<8)+$reg_l));
    max_shutter=`echo $reg_val $time_1h | awk '{printf "%d\n",$1*$2}'`
    printf "r max_shutter is %d us\n" $max_shutter;
}

write_auto_shutter_max()
{
    local regval=0;
    local time_1h=0;
    local max_shutter=0;
    local res=0;
    local reg_l=0;
    local reg_h=0;
    local reg_val=0;
    local res=0;
    __read_videoformat;
    
    if [ $VIDEO_FORMAT -eq 1 ] ; then
		printf "r Video Format is NTSC(60Hz)\n";
        time_1h=14.815;#us
	elif [ $VIDEO_FORMAT -eq 0 ] ; then
		printf "r Video Format is PAL(50Hz)\n";
        time_1h=17.778;#us
	fi
    
    max_shutter=$PARAM1;
    reg_val=`echo $max_shutter $time_1h | awk '{printf "%d\n",$1/$2}'`
    if [ $reg_val -gt 2250 ] ; then
        reg_val=2250;
        printf "max_shutter time too long,will cut it!\n";
    fi
    reg_h=$(($reg_val>>8));
    reg_l=$(($reg_val&0xFF));
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x04 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $reg_h);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x07 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $reg_l);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    printf "w max_shutter %d us\n" $max_shutter;
}

read_exptime()
{
    local regval=0;
    local expreg_l=0;
    local expreg_h=0;
	local res=0;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x12);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	expreg_h=$?;
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x13);
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01);
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	expreg_l=$?;
    #exptime=expreg*29.6us@30fps mode
	regval=$((($expreg_h<<8)+$expreg_l));
	exptime=`echo $regval 29.6 | awk '{printf "%d\n",$1*$2}'`
	printf "r reg 0x%x, exptime is %d us\n" $regval $exptime;
    
}

read_osd()
{
    local regval_0=0;
    local regval_1=0;
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDF );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x58 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
	sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
    regval_0=$?;
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0x47 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x5A );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
    sleep 0.01;
	res=$(i2c_read $I2C_DEV $I2C_ADDR  0x14 );
    regval_1=$?;
    printf "r close osd 0x%x, 0x%x\n" $regval_0 $regval_1;
}

write_osd()
{
 	local regval_0=0;
    local regval_1=0;
	if [ $PARAM1 -eq 1 ] ; then
		printf "w open osd\n";
		regval_0=0xA9;
		regval_1=0xB0;
	else
        printf "w close osd\n";
		regval_0=0xA8;
		regval_1=0x30;
	fi
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDF );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x58 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $regval_0 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
    sleep 0.01;
    res=$(i2c_write $I2C_DEV $I2C_ADDR  0x10 0x47 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x11 0x5A );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x12 $regval_1 );
	res=$(i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
}
#######################Action# BEGIN##############################

pinmux;
i2c_write $I2C_DEV $I2C_ADDR  0x07 0xFE>/dev/null 2>&1

if [ ${MODE} = "read" ] ; then
	case $FUNCTION in
		"devid"|"deviceid")
			read_devid;
			;;
		"hdver"|"hardwareversion")
			read_hardver;
			;;
        "sensorid")
            read_sensorid;
            ;;
		"wdrmode")
			read_wdrmode;
			;;
		"videoformat")
			read_videoformat;
			;;
		"mirrormode")
			read_mirrormode;
			;;
		"denoise")
			read_denoise;
			;;
		"agc")
			read_agc;
			;;
		"lowlight")
			read_lowlight;
			;;
		"daynightmode")
			read_daynightmode;
			;;
		"ircutdir")
			read_ircutdir;
			;;
		"irtrigger")
			read_irtrigger;
			;;
		"mshutter")
			read_mshutter;
			;;
        "cameramode")
			read_cameramode;
			;;
        "nodf")
			read_nodf;
			;;
        "csienable")
			read_csienable;
			;;
        "saturation")
			read_saturation;
			;;
        "wdrbtargetbr")
			read_wdrbtargetbr;
			;;
        "brightness")
			read_brightness;
			;;
        "aespeed")
			read_aespeed;
			;;
        "contrast")
            read_contrast;
            ;;
        "satu")
            read_satu;
            ;;
        "sharppen")
            read_sharppen;
            ;;
        "wdrsharppen")
            read_wdrsharppen;
            ;;
        "wdrtargetbr")
			read_wdrtargetbr;
			;;
        "lsc")
			read_lsc;
			;;
        "lsc_slop")
			read_lsc_slop;
			;;
        "boardmodel")
			read_board_model;
			;;
        "i2cauxenable")
			read_i2c_aux_enable;
			;;
        "i2cwen")
			read_i2c_write_enable;
			;;
        "awbgain")
			read_awbgain;
			;;
        "wbmode")
			read_wbmode;
			;;
        "mwbgain")
			read_mwbgain;
			;;
        "yuvseq")
            read_yuvseq;
                ;;
        "antiflicker")
            read_antiflicker;
                ;;
        "defog")
            read_defog;
                ;;
        "blcstrength")
            read_blcstrength;
                ;;
        "blcpos")
            read_blcpos;
                ;;
        "new_expmode")
            read_new_expmode;
                ;;
        "new_mshutter")
            read_new_mshutter;
                ;;
        "new_mgain")
            read_new_mgain;
                ;;
        "auto_shutter_max")
            read_auto_shutter_max;
                ;;
        "exptime")
            read_exptime;
                ;;
        "osd")
            read_osd;
                ;;
	esac
fi


if [ ${MODE} = "write" ] ; then
	case $FUNCTION in
		"devid"|"deviceid")
			echo "NOT SUPPORTED!";
			;;
		"hdver"|"hardwareversion")
			echo "NOT SUPPORTED!";
			;;
		"wdrmode")
			write_wdrmode;
			;;
		"videoformat")
			write_videoformat;
			;;
		"mirrormode")
			write_mirrormode;
			;;
		"denoise")
			write_denoise;
			;;
		"agc")
			write_agc;
			;;
		"lowlight")
			write_lowlight;
			;;
		"daynightmode")
			write_daynightmode;
			;;
		"ircutdir")
			write_ircutdir;
			;;
		"irtrigger")
			write_irtrigger;
			;;
		"mshutter")
			write_mshutter;
			;;
        "cameramode")
			write_cameramode;
			;;
        "nodf")
			write_nodf;
			;;
        "capture")
			write_capture;
			;;
        "csienable")
			write_csienable;
			;;
        "saturation")
			write_saturation;
			;;
        "wdrbtargetbr")
			write_wdrbtargetbr;
			;;
        "brightness")
			write_brightness;
			;;
        "aespeed")
			write_aespeed;
			;;
        "contrast")
            write_contrast;
            ;;
        "satu")
            write_satu;
            ;;
        "sharppen")
            write_sharppen;
            ;;
        "wdrsharppen")
            write_wdrsharppen;
            ;;
        "wdrtargetbr")
            write_wdrtargetbr;
			;;
        "lsc")
			write_lsc;
			;;
        "lsc_slop")
			write_lsc_slop;
			;;
        "i2cauxenable")
			write_i2c_aux_enable;
			;;
        "i2cwen")
			write_i2c_write_enable;
			;;
        "wbmode")
			write_wbmode;
			;;
        "mwbgain")
			write_mwbgain;
			;;
        "awbexpt")
                write_awbexpt;
                ;;
        "yuvseq")
            write_yuvseq;
                ;;
        "antiflicker")
            write_antiflicker;
                ;;
        "defog")
            write_defog;
                ;;
        "blcstrength")
            write_blcstrength;
                ;;
        "blcpos")
            write_blcpos;
                ;;
        "paramsave")
            write_ispparamsave;
                ;;
        "new_expmode")
            write_new_expmode;
                ;;
        "new_mshutter")
            write_new_mshutter;
                ;;
        "new_mgain")
            write_new_mgain;
                ;;
        "auto_shutter_max")
            write_auto_shutter_max;
                ;;
        "osd")
            write_osd;
                ;;
	esac
    sleep 0.1;
fi

#disable i2c transfer
i2c_write $I2C_DEV $I2C_ADDR  0x07 0xFF>/dev/null 2>&1
