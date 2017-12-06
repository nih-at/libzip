# expect variables IN, OUT, and DIR
EXECUTE_PROCESS(COMMAND mandoc -T html -Oman="%N.html",style=../nih-man.css ${DIR}/${IN}
  COMMAND ${DIR}/fix-man-links.sh
  OUTPUT_FILE ${DIR}/${OUT}.new)
CONFIGURE_FILE(${DIR}/${OUT}.new ${DIR}/${OUT} COPYONLY)
FILE(REMOVE ${DIR}/${OUT}.new)

