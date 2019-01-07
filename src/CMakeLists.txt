
set(SOURCES
	${SOURCES_PREFIX}/sl_config.h
	${SOURCES_PREFIX}/board_config.c
	${SOURCES_PREFIX}/board_trace.c
  ${SOURCES_PREFIX}/board_trace.h
	${SOURCES_PREFIX}/link_transport_uart.c
	${SOURCES_PREFIX}/link_transport_uart.h
  ${SOURCES_PREFIX}/link_transport_usb.c
  ${SOURCES_PREFIX}/link_transport_usb.h
  ${SOURCES_PREFIX}/localfs.c
  ${SOURCES_PREFIX}/localfs.h
  ${SOURCES_PREFIX}/semihost_api.c
  ${SOURCES_PREFIX}/semihost_api.h
  ${SOURCES_PREFIX}/stratify_symbols.c
  PARENT_SCOPE)
