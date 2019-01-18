source rx_functions.sh
source tx_functions.sh

function MAIN_TX_RX_FUNCTION {
	if [ "$CAM" == "0" ]; then
	    rx_function
	else
	    tx_function
	fi
}
