#include <main.h>
#include <string.h>

MenuItem *current; // The currently active MenuItem
FILE *port; // The port that the LCD is connected to

// Private functions

/**
* Runs a motor at 127 forward and back for half a second each
*
* @param port is the motor port to pulse
*/
void pulseMotor(unsigned char port);

/**
* Runs a Power-On-Self-Test (pulses all motors and bleeps speaker when done)
*/
void runSelfTest();

/**
* Switches to autonomous mode
*/
void callAuton();
void recordAuton(int recordSlot);
void replayAuton(int replaySlot);
void autonSel();

MenuItem *lcdmInit(FILE *lcdPort) {
  port = lcdPort;
  // Initialize LCD
  lcdInit(lcdPort);
  lcdClear(lcdPort);
  lcdSetBacklight(lcdPort, true);
  // Set Default text
  lcdSetText(lcdPort, 1, "~~750Evolution~~");
  lcdSetText(lcdPort, 2, "Initializing...");
  printf("LCD Initialized. Creating root menu...\n\r");
  delay(20);

  // Initialize the MenuItem linked list
  MenuItem *root = malloc(sizeof(MenuItem *));
  root = lcdmCreateItem("<   Select     >");
  // Make list circular for adding items later
  (*root).previous = root;
  (*root).next = root;
  printf("Menu created. Setting active menu...\n\r");
  delay(20);
  // Start up menu TODO: Call loop in separate task?
  current = root;
  printf("LCD Menu Initialized\n\r");
  delay(20);
  return root;
}

void lcdmAddDefaults(MenuItem *root) {
  printf("Adding default LCDM features...\n\r");
  delay(20);
  printf("Creating Manual Test menu...\n\r");
  delay(20);
  // Create Manual Test entry on main menu
  MenuItem *manualTest = lcdmCraddItem("< Manual Test  >", root, NULL, NULL);
  // Create Manual Test submenu (item that links back to main menu)
  MenuItem *exitManualTest = lcdmCraddSubmenu(manualTest);

  printf("Adding Manual Test submenus...\n\r");
  delay(20);
  char *title;
  // Adds an item for each motor port
  for (int i = 1; i<=10; i++) {
    title = malloc(sizeof(char)*16);
    sprintf(title, "<   Motor %02d   >",i);
    delay(20);
    // Creates/adds the menu item that will call pulseMotor
    MenuItem *motor = lcdmCraddItem(title, exitManualTest, &pulseMotor, NULL);
    // Parameter needs to be set separately to avoid type errors (TODO: Figure out why)
    motor->param = i;
    printf("%s", (*motor).name);
  }

  //Create speaker test item and add to Manual Test submenu
  MenuItem *speaker = lcdmCraddItem("<   Speaker    >", exitManualTest, &beep, NULL);
  // Create POST test menu item
  lcdmCraddItem("<     POST     >", root, &runSelfTest, NULL);
  //Create autonomous menu item
  lcdmCraddItem("<  Autonomous  >", root, &callAuton, NULL);

  printf("Creating Auton Recorder menu...\n");
  delay(20);
  MenuItem *autonRecorder = lcdmCraddItem("<  Make Auton  >", root, NULL, NULL);
  MenuItem *exitAutonRecorder = lcdmCraddSubmenu(autonRecorder);

  printf("Adding Auton Recorder submenu...\n");
  delay(20);

  for(int x = 1; x<=8; x++){
    title = malloc(sizeof(char)*16);
    sprintf(title,"<    Slot %d    >",x);
    delay(20);
    MenuItem *autonSlot = lcdmCraddItem(title,exitAutonRecorder, &recordAuton, NULL);
    autonSlot->param = x;
    printf("%s",(*autonSlot).name);
  }

  printf("Creating Auton Replayer menu...\n");
  delay(200);
  MenuItem *autonReplayer = lcdmCraddItem("<  Play Auton  >", root, NULL, NULL);
  MenuItem *exitAutonReplayer = lcdmCraddSubmenu(autonReplayer);

  printf("Adding Auton Replayer submenu...\n");
  delay(20);

  for(int x = 1; x<=8; x++){
    title = malloc(sizeof(char)*16);
    sprintf(title,"<    Slot %d    >",x);
    delay(20);
    MenuItem *autonSlot = lcdmCraddItem(title,exitAutonReplayer, &replayAuton, NULL);
    autonSlot->param = x;
    printf("%s",(*autonSlot).name);
  }

  // speaker->param = 0;
  printf("Done.\n\r");
  delay(20);
  lcdmCraddItem("<  Auton Sel   >", lcdRoot, &autonSel, NULL);

}

MenuItem *lcdmCraddItem(char *name, MenuItem *root, void *action, void *param) {
  // Create item to be added
  MenuItem *addee = malloc(sizeof(MenuItem *));
  addee = lcdmCreateItem(name);
  lcdmAddItem(root, addee);
  addee->action = action;
  // Return pointer to item
  return addee;
}

MenuItem *lcdmCraddSubmenu(MenuItem *link) {
  // Create "Back" item (root of submenu)
  MenuItem *submenu = lcdmCraddItem("<     Back     >", NULL, NULL, NULL);
  // Make it circular
  submenu->next = submenu;
  submenu->previous = submenu;
  // Link main menu entry to submenu
  link->select = submenu;
  // Link submenu back to main menu
  submenu->select = link;
  return submenu;
}

MenuItem *lcdmCreateItem(char *name) {
  // Initialize MenuItem fields
  MenuItem item = { .name = name,
                    .previous = malloc(sizeof(MenuItem *)),
                    .next = malloc(sizeof(MenuItem *)),
                    .select = malloc(sizeof(MenuItem *)),
                    .action = malloc(sizeof(void *)),
                    .param = malloc(sizeof(void *))
                  };
  // Set pointers to NULL
  item.select = NULL;
  item.action = NULL;
  item.param = NULL;
  // Clone MenuItem to persistent memory location
  // (prevents it from being deleted since item is within the scope of this function)
  // Returning &item would cause a null pointer issue and thus a cortex crash
  MenuItem *clone = malloc(sizeof(MenuItem *));
  *clone = item;

  return clone;
}

void lcdmAddItem(MenuItem *root, MenuItem *item) {
  // Make sure not to add null items to menu (prevent runtime crashes due to code errors)
  if(root == NULL || item == NULL) {
    return;
  }
  // Link all the items together (effectively inserts the item before the root and closes the circle in both directions)
  MenuItem *last = (*root).previous;
  (*last).next = item;
  (*root).previous = item;
  (*item).next = root;
  (*item).previous = last;
}

void lcdmLoop(MenuItem *root) {
  // printf("loop: %p\n\r",current);
  // printf("Current: %s\n\r",(*current).name);

  lcdSetText(port, 2, (*current).name);

  // Check buttons
  if(lcdReadButtons(port) == LCD_BTN_LEFT) {
    current = (*current).previous;
  } else if (lcdReadButtons(port) == LCD_BTN_RIGHT) {
    current = (*current).next;
  } else if (lcdReadButtons(port) == LCD_BTN_CENTER) {
    if((*current).select > 0) {
      // If selected item is linked to submenu, navigate to that submenu
      current = (*current).select;
    } else if((*current).action > 0) {
      // If selected item is linked to function, run that function with saved parameters
      (*current).action((*current).param);
    } else {
      printf("Nothing to do for %s\n\r", (*current).name);
    }
  }
}

/**
 * Action Functions:
 */

void pulseMotor(unsigned char port) {
  // Pulse motor in both directions and stop
  motorSet(port, 127);
  delay(500);
  motorSet(port, -127);
  delay(500);
  motorStop(port);
}

void beep() {
  // F, A# (aka b-flat)
  speakerPlayRtttl("Beep:d=4,o=5,b=300:16f5");
  speakerPlayRtttl("Beep:d=4,o=5,b=300:16a#6");
}

void boop() {
  // A# (aka b-flat), F
  speakerPlayRtttl("Beep:d=4,o=5,b=300:16a#6");
  speakerPlayRtttl("Beep:d=4,o=5,b=300:16f5");
}

void runSelfTest() {
  lcdSetText(port, 2, "POST: DONT TOUCH");
  // Pulse all motors
  for(int i = 1; i <= 10; i++) {
    pulseMotor(i);
  }
  beep();
  beep();
}

void callAuton() {
  // Add warning to LCD before calling auton
  lcdSetText(port, 2, "AUTON-DONT TOUCH");
  autonomous();
}

void recordAuton(int recordSlot) {
  lcdSetText(port, 2, "Recording Auton ");
  startRecording(recordSlot);
  delay(500);
}

void replayAuton(int replaySlot) {
  lcdSetText(port, 2, "Playing auton ");
  setActiveSlot(replaySlot);
  autonomous();
}


void autonSel(){
	char * string = malloc(sizeof(char)*16);
	sprintf(string,"Pot val: %d",analogRead(POT));
	lcdSetText(uart2,1,"Auton Sel");
	lcdSetText(uart2,2,string);
	free(string);
}
