
int jogWithButtons() {
	if(!digitalRead(BUTTONF)) {
		s.setSpeed(JOG_SPEED);
		s.runSpeed();
	}
	else if(!digitalRead(BUTTONR)) {
		s.setSpeed(-JOG_SPEED);
		s.runSpeed();
	}
	else{
		s.setSpeed(0);
	}

	if(db(BUTTONSEL)){
		return -1;
	}

	return 0;
}

void setLED(bool g, bool y, bool r){
	digitalWrite(LEDG, g);
	digitalWrite(LEDY, y);
	digitalWrite(LEDR, r);
}

void getLine() {
	bool addNewline = false;
	if(!dataFile.available()) {
		#ifdef DEBUGSERIAL
		Serial.println("There is no remaining text in the file. Appending a possibly redundant newline.");
		#endif
		addNewline = true;
	}

	char c = ' ';    //some irrelevant starting value
	byte i = 0; //counter

	int x;
	for(x = 0; x < MAX_LINE_BYTES; x++) {
		dataLine[x] = 0x00;
	}
	if(addNewline) {
		dataLine[x+1] = '\n';
	}

	while (dataFile.available() && c != '\n' && i < MAX_LINE_BYTES) { //while there is data to be read
		c = dataFile.read(); //read in the data
		#ifdef DEBUGSERIAL
		Serial.print("New character loaded: ");
		Serial.println(c);
		#endif
		dataLine[i] = c;
		i++;
	}
}

void activePumpingLoop() {	//This accounts for 99% of runtime while running SD pump commands
	s.runSpeed();
}

void setQ(float q) {
	float stepSpeed = UL_PER_STEP * q;
	#ifdef DEBUGSERIAL
	Serial.print("Set q: ");
	Serial.print(q);
	Serial.print(" Set speed: ");
	Serial.println(stepSpeed);
	#endif
	s.setSpeed(stepSpeed * TUNE);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void initSD(){

	do{
		delay(300);	//Sd card no likey rapid reinits
		if (!SD.begin(SDCS)) {
			setLED(0, 0, 1);
			#ifndef NOSERIAL
			Serial.println("Couldn't reach SD card...");
			#endif
		}
		delay(300);
		dataFile = SD.open(CMDFILE);
		if (!dataFile) {
			setLED(0, 0, 1);
			#ifndef NOSERIAL
			Serial.println("commands.txt not found...");
			#endif
		}
	} while(!dataFile.available());
	setLED(0, 0, 0);
}

bool db(byte pin){	//Make buttons sane
	if(millis() - lastSel < DB_THRESH){
		return false;	//don't accept if pushed too fast since last
	}
	if(!digitalRead(pin)){
		lastSel = millis();
		setLED(1, 1, 1);
		while(!digitalRead(pin)){

		}
		setLED(0, 0, 0);
		if(millis() - lastSel < DB_THRESH){	//If not enough time has passed
			return false;
		} else{
			lastSel = millis();	//successful button press
			return true;
		}
	}
	return false;
}

void setStepRate(byte rate){
	switch(rate) {
		case 1:
		setMode(0, 0, 0);
		break;
		case 2:
		setMode(1, 0, 0);
		break;
		case 4:
		setMode(0, 1, 0);
		break;
		case 8:
		setMode(1, 1, 0);
		break;
		case 16:
		setMode(0, 0, 1);
		break;
		case 32:
		setMode(1, 1, 1);
		break;
		default:
		setMode(0, 0, 0);
		break;
	}
}

void setMode(bool p0, bool p1, bool p2) {
	digitalWrite(MODE0, p0);
	digitalWrite(MODE1, p1);
	digitalWrite(MODE2, p2);
}

int pump(){
	#ifdef DEBUGSERIAL
	Serial.println("================= Pumping loop begin:");
	#endif
	getLine(); //get the next line

	if (strstr(dataLine, flagQA)) {   //if command is for gradient (if qa flag exists)
		float ta;
		float qa;
		float tb;
		float qb;

		#ifdef DEBUGSERIAL
		Serial.println("Begin: GRADIENT command parsing");
		Serial.println("Command: ");
		Serial.println(dataLine);
		#endif

		//Get data
		ta = atof(strstr(dataLine, flagTA) + 2);    //get buff @ the index of the flag buff, then parse it and return a float.
		qa = atof(strstr(dataLine, flagQA) + 2);
		tb = atof(strstr(dataLine, flagTB) + 2);
		qb = atof(strstr(dataLine, flagQB) + 2);

		#ifdef DEBUGSERIAL
		Serial.print("Command index: ");
		Serial.println(commandIndex);
		Serial.println(ta);
		Serial.println(qa);
		Serial.println(tb);
		Serial.println(qb);
		#endif

		if (ta == 0 && commandIndex != 0) { //Cheap way of figuring out if String.toFloat failed
			#ifdef DEBUGSERIAL
			Serial.println("This command has stopped the routine.");
			#endif
			return -1;	//Tell loop() that pumping has completed
		}

		while (1) {	//wait for the beginning of the gradient to start
			if (millis() - offsetTime >= ta * 1000) { //if the queued event is up or has passed
				break;
			}
			activePumpingLoop();  //While we wait, do all the things.
			if(db(BUTTONSEL)){
				return -1;	//if the button is pushed, we tell loop() to go to jog mode
			}
		}

		unsigned long startTime = offsetTime + (ta * 1000); //start of the gradient where it SHOULD be
		while (1) { //Loop for the duration of the gradient command

			if (recalculationInterval.trigger()) {
				float newQ = (float) mapFloat((millis() - startTime), 0.0, (tb - ta) * 1000.0, qa, qb);
				setQ(newQ);
			}

			if ((millis() - startTime) > (tb - ta) * 1000) { //if the gradient is complete
				break;
			}
			activePumpingLoop();    //While we wait, do all the things.
			if(db(BUTTONSEL)){
				return -1;	//if the button is pushed, we tell loop() to go to jog mode
			}
		}
	} else {
		float t;
		float q;

		#ifdef DEBUGSERIAL
		Serial.println("Begin: NORMAL command parsing");
		Serial.print("Command: ");
		Serial.println(dataLine);
		#endif

		t = atof(strchr(dataLine, flagT) + 1);
		q = atof(strchr(dataLine, flagQ) + 1);

		#ifdef DEBUGSERIAL
		Serial.print("Command index: ");
		Serial.println(commandIndex);
		Serial.println(t);
		Serial.println(q);
		#endif

		if (t == 0 && commandIndex != 0) { //Cheap way of figuring out if String.toFloat failed
			#ifdef DEBUGSERIAL
			Serial.println("This command has stopped the routine.");
			#endif
			return -1;	//Tell loop() that pumping has completed
		}

		while (1) {	//wait for the event to be up
			if (millis() - offsetTime >= t * 1000) { //if the queued event is up or has passed
				break;
			}
			activePumpingLoop();    //While we wait, do all the things.
			if(db(BUTTONSEL)){
				return -1;	//if the button is pushed, we tell loop() to go to jog mode
			}
		}
		setQ(q);
	}

	commandIndex++;
	return 0;	//tell loop() that we're not finished pumping
}
