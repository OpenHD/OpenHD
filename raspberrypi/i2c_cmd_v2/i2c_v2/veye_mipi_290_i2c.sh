
#!/bin/sh

#
#set camera i2c pin mux
#

I2C_DEV=0;
I2C_ADDR=0x3b;

print_usage()
{
	echo "Usage:  ./veye_mipi_290_isp.sh [-r/w] [-f] function name -p1 param1 -p2 param2 "
	echo "options:"
	echo "    -r                       read "
	echo "    -w                       write"
	echo "    -f [function name]       function name"
	echo "    -p1 [param1] 			   param1 of each function"
	echo "    -p2 [param1] 			   param2 of each function"
	echo -e "function list and param,ref to [veye_mipi_290_isp_function_and_param.pdf]"
	echo "support functions: devid,hdver,wdrmode,videoformat,mirrormode,denoise,agc,lowlight,daynightmode,ircutdir,irtrigger"
}
######################parse arg###################################
MODE=read;
FUNCTION=version;
PARAM1=0;
PARAM2=0;
b_arg_param1=0;
b_arg_param2=0;
b_arg_functin=0;

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
	sh ./camera_i2c_config &> /dev/null
}

read_devid()
{
	local verid=0;
	local res=0;
	res=$(./i2c_read $I2C_DEV $I2C_ADDR  0x00 );
	verid=$?;
	printf "device id is 0x%2x\n" $verid;
}

read_hardver()
{
	local hardver=0;
	local res=0;
	res=$(./i2c_read $I2C_DEV $I2C_ADDR  0x01 );
	hardver=$?;
	printf "hardware version is 0x%2x\n" $hardver;
}

read_wdrmode()
{
	local wdrmode=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDB );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x11 0x32 );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
	res=$(./i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	wdrmode=$?;
	printf "r wdrmode is 0x%2x\n" $wdrmode;
}
write_wdrmode()
{
	local wdrmode=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDB );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x11 0x32 );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w wdrmode is 0x%2x\n" $PARAM1;
}

read_videoformat()
{
	local videoformat=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDE );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x11 0xC2 );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
	res=$(./i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	videoformat=$?;
	echo "frame rate reg "$videoformat;
	if [ $videoformat -eq 1 ] ; then
		printf "r Video Format is NTSC(60Hz)\n";
	fi
	if [ $videoformat -eq 0 ] ; then
		printf "r Video Format is PAL(50Hz)\n";
	fi
}

write_videoformat()
{
	local videoformat=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDE );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x11 0xC2 );
	if [ $PARAM1 == "PAL" ] ; then
		res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x12 0x0);
	fi
	if [ $PARAM1 == "NTSC" ] ; then
		res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x12 0x1);
	fi
	
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w videoformat is %s\n" $PARAM1;
}

read_mirrormode()
{
	local mirrormode=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDE );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x11 0x57 );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
	res=$(./i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	mirrormode=$?;
	printf "r mirrormode is 0x%2x\n" $mirrormode;
}
write_mirrormode()
{
	local mirrormode=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDE );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x11 0x57 );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w mirrormode is 0x%2x\n" $PARAM1;
}

read_denoise()
{
	local denoise=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD8 );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x11 0x9B );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
	res=$(./i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	denoise=$?;
	printf "r denoise is 0x%2x\n" $denoise;
}
write_denoise()
{
	local denoise=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x10 0xD8 );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x11 0x9B );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w denoise is 0x%2x\n" $PARAM1;
}

read_agc()
{
	local agc=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x11 0x67 );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
	res=$(./i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	agc=$?;
	printf "r agc is 0x%2x\n" $agc;
}
write_agc()
{
	local agc=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x11 0x67 );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w agc is 0x%2x\n" $PARAM1;
}

read_lowlight()
{
	local lowlight=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x11 0x64 );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x13 0x01 );
	res=$(./i2c_read $I2C_DEV $I2C_ADDR  0x14 );
	lowlight=$?;
	printf "r lowlight is 0x%2x\n" $lowlight;
}
write_lowlight()
{
	local lowlight=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x10 0xDA );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x11 0x64 );
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x12 $PARAM1);
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x13 0x00 );
	printf "w lowlight is 0x%2x\n" $PARAM1;
}

read_daynightmode()
{
	local daynightmode=0;
	local res=0;
	res=$(./i2c_read $I2C_DEV $I2C_ADDR  0x02 );
	daynightmode=$?;
	printf "r daynightmode is 0x%2x\n" $daynightmode;
}
write_daynightmode()
{
	local daynightmode=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x02 $PARAM1);
	printf "w daynightmode is 0x%2x\n" $PARAM1;
}
read_ircutdir()
{
	local ircutdir=0;
	local res=0;
	res=$(./i2c_read $I2C_DEV $I2C_ADDR  0x16 );
	ircutdir=$?;
	printf "r ircutdir is 0x%2x\n" $ircutdir;
}
write_ircutdir()
{
	local ircutdir=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR  0x16 $PARAM1);
	printf "w ircutdir is 0x%2x\n" $PARAM1;
}

read_irtrigger()
{
	local irtrigger=0;
	local res=0;
	res=$(./i2c_read $I2C_DEV $I2C_ADDR  0x15 );
	irtrigger=$?;
	printf "r irtrigger is 0x%2x\n" $irtrigger;
}
write_irtrigger()
{
	local irtrigger=0;
	local res=0;
	res=$(./i2c_write $I2C_DEV $I2C_ADDR 0x15 $PARAM1);
	printf "w irtrigger is 0x%2x\n" $PARAM1;
}


#######################Action# BEGIN##############################

pinmux;
./i2c_write $I2C_DEV $I2C_ADDR  0x07 0xFE&> /dev/null;

if [ ${MODE} == "read" ] ; then	
	case $FUNCTION in
		"devid"|"deviceid")
			read_devid;
			;;
		"hdver"|"hardwareversion")
			read_hardver;
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
	esac
fi



if [ ${MODE} == "write" ] ; then 	
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
	esac
fi


