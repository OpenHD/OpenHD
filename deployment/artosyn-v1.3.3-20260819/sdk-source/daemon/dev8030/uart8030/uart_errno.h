#ifndef __UART_ERRNO__
#define __UART_ERRNO__

#ifdef __cplusplus
extern "C"
{
#endif

/* errno of the usr return */
typedef enum uart_ret_e {
    UART_ERROR_NONE = 0,             /** No error*/
    UART_ERROR_PARAM = -10000,       /** Invalid param */
    UART_ERROR_MEM,                  /** No memory */
    UART_ERROR_EMPTY,                /** Buffer is empty */

    UART_ERROR_PKT_NOT_COMP,         /** Pkt rcv not complete */
    UART_ERROR_PKT_INVALID_LEN,      /** Pkt with invalid length */
    UART_ERROR_PKT_INVALID_MAGIC,    /** Pkt with invalid magic number */
    UART_ERROR_PKT_DROP              /** Generate a dropping pkt for rb free */
} uart_ret;

#ifdef __cplusplus
}
#endif

#endif