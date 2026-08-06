##
## Temporary automated reproducer for the table-console sticky-scroll bug.
## Scripts the user interaction (tab switch, timeline drag, tab return) with
## synthesized input events and prints the QConsoleWidget state transitions.
##

set(QT_MODULES
    Core
    Widgets
    Gui
    Test
)

list(APPEND ADDITIONAL_LIBRARIES )

exampleMaster(${LIBRARY_NAME} ${LIB_PROFILE_DEFINE} ${QT_ENABLE} ${QT_DEPLOY} "${QT_MODULES}" "${ADDITONAL_SOURCES}" "${ADDITIONAL_LIBRARIES}" "${INSTALL_BIN_PATH}")
