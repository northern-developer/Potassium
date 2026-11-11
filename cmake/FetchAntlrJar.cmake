# Invoked via `cmake -P` from the regen-parser custom target.
# Expects ANTLR_JAR, ANTLR_JAR_VERSION, ANTLR_JAR_SHA256 on the command line (-D).
if(NOT EXISTS "${ANTLR_JAR}")
    file(DOWNLOAD
        "https://www.antlr.org/download/antlr-${ANTLR_JAR_VERSION}-complete.jar"
        "${ANTLR_JAR}"
        EXPECTED_HASH "SHA256=${ANTLR_JAR_SHA256}"
    )
endif()
