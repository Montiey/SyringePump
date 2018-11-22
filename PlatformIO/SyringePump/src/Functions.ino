
int jogWithButtons() {
	if(!digitalRead(BUTTONF)) {
		if(!jogHeld){
			if(db_hold(BUTTONF)) jogHeld = true;	//skip hold check for next presses
			return 0;
		} else{
			s.setSpeed(JOG_SPEED * configTNR);
			return 0;
		}
	}

	if(!digitalRead(BUTTONR)) {
		if(!jogHeld){
			if(db_hold(BUTTONR)) jogHeld = true;	//skip hold check for next presses
			return 0;
		} else{
			s.setSpeed(-JOG_SPEED * configTNR);
			return 0;
		}
	}

	//	Control enters here if no buttons are pressed

	s.setSpeed(0);	//Set if no buttons because s.runSpeed() is called all the time in main()
	jogHeld = false;	//Recheck the buttons for holding

	if(db(BUTTONSEL)){
		return -1;
	}

	return 0;
}

void setLED(byte color){
	currentColor = color;
	bool r = 0;
	bool g = 0;
	bool b = 0;
	switch(color){
		case RED:
		r = 1;
		break;
		case GREEN:
		g = 1;
		break;
		case YELLOW:
		r = 1; g = 1;
		break;
		case BLUE:
		b = 1;
		break;
		case MAGENTA:
		r = 1; b = 1;
		break;
		case CYAN:
		g = 1; b = 1;
		break;
		case WHITE:
		r = 1; g = 1; b = 1;
		break;
	}

	digitalWrite(LEDR, !r);
	digitalWrite(LEDG, !g);
	digitalWrite(LEDB, !b);
}

void getLine() {
	bool addNewline = false;
	if(!dataFile.available()) {
		#ifndef NODEBUG
		Serial.println("End of file");
		#endif
		addNewline = true;
	}

	char c = ' ';	//Some irrelevant starting value
	byte i = 0;	//Counter

	int x;
	for(x = 0; x < MAX_LINE_BYTES; x++) {
		dataLine[x] = 0x00;
	}
	if(addNewline) {
		dataLine[x+1] = '\n';
	}

	while (dataFile.available() && c != '\n' && i < MAX_LINE_BYTES) {	//While there is data to be read
		c = dataFile.read();	//Read in the data
		#ifndef NODEBUG
		Serial.print("New character loaded: ");
		Serial.println(c);
		#endif
		dataLine[i] = c;
		i++;
	}
}

void activePumpingLoop() {	//This accounts for most of runtime
	s.runSpeed();
	if(LEDTimer.trigger()){
		if(currentColor){
			setLED(BLACK);
		} else{
			setLED(GREEN);
		}
	}
}

void setQ(float q) {
	float stepSpeed = ULPerStep * q * configTN;
	#ifndef NODEBUG
	Serial.print("Set q: ");
	Serial.print(q);
	Serial.print(" Set speed: ");
	Serial.println(stepSpeed);
	#endif
	s.setSpeed(stepSpeed);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void initSD(){
	#ifndef NOSERIAL
	Serial.println("SD init");
	#endif

	File configFile;
	do{
		if (!SD.begin(SDCS)) {
			setLED(RED);
			#ifndef NOSERIAL
			Serial.println("SD read error");
			#endif
			delay(300);
		}
		dataFile = SD.open(CMDFILE);
		if (!dataFile) {
			setLED(RED);
			#ifndef NOSERIAL
			Serial.print(CMDFILE);
			Serial.println(" file error");
			#endif
			delay(300);
		}




		configFile = SD.open(CONFIGFILE);



		if(!configFile){
			setLED(RED);
			#ifndef NOSERIAL
			Serial.print(CONFIGFILE);
			Serial.println(" file error");
			#endif
			delay(300);
		} else {	//If we can access it, go ahead and do some stuff
			char * configText = (char *) calloc(MAX_CONFIG_BYTES, 1);	//config is loaded all at once
			int i = 0;
			while(configFile.available() && i < MAX_CONFIG_BYTES){
				configText[i] = configFile.read();;
				i++;
			}
			#ifndef NOSERIAL
			Serial.print("Text: ");
			Serial.println(configText);
			#endif

			configID = atof(strstr(configText, flagID) + strlen(flagID));
			configTN = atof(strstr(configText, flagTN) + strlen(flagID));
			if(configTN > 0){
				configTNR = 1;
			} else{
				configTNR = -1;
			}
			ULPerStep = PITCH * (360.0 / (STEPS * USTEP_RATE) ) * (PI * (float)(pow(configID/2.0, 2.0)));

			#ifndef NOSERIAL
			Serial.print("UL per step: ");
			Serial.println(ULPerStep);
			#endif

			configFile.close();
		}
	} while(!dataFile.available());
	setLED(BLACK);
	#ifndef NOSERIAL
	Serial.println("SD init done");
	#endif
}

bool db(byte pin){	//Button debounce (WAITS FOR RELEASE)
	byte lastColor = currentColor;
	if(millis() - lastSel < DB_THRESH){
		setLED(lastColor);
		return false;	//Don't accept if pushed too fast since last
	}
	if(!digitalRead(pin)){
		lastSel = millis();
		setLED(WHITE);
		while(!digitalRead(pin)){

		}
		setLED(BLACK);
		if(millis() - lastSel < DB_THRESH){	//If not enough time has passed
			setLED(lastColor);
			return false;
		} else{
			lastSel = millis();	//Successful button press
			setLED(lastColor);
			return true;
		}
	}
	setLED(lastColor);
	return false;
}

bool db_hold(byte pin){	//Button hold debounce (DOESN'T WAIT FOR RELEASE)
	unsigned long start = millis();
	while(!digitalRead(pin)){
		if(millis() - start >= DB_HOLD_THRESH){
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
	#ifndef NODEBUG
	Serial.println("================= Pumping loop begin:");
	#endif
	getLine();	//Get the next line

	if (strstr(dataLine, flagQA)) {	//If command is for gradient (if qa flag exists)
		float ta;
		float qa;
		float tb;
		float qb;

		#ifndef NODEBUG
		Serial.println("Begin: GRADIENT command parsing");
		Serial.println("Command: ");
		Serial.println(dataLine);
		#endif

		//Get data
		ta = atof(strstr(dataLine, flagTA) + strlen(flagTA));	//Get buff @ the index of the flag buff, then parse it and return a float.
		qa = atof(strstr(dataLine, flagQA) + strlen(flagQA));
		tb = atof(strstr(dataLine, flagTB) + strlen(flagTB));
		qb = atof(strstr(dataLine, flagQB) + strlen(flagQB));

		#ifndef NODEBUG
		Serial.print("Command index: ");
		Serial.println(commandIndex);
		Serial.println(ta);
		Serial.println(qa);
		Serial.println(tb);
		Serial.println(qb);
		#endif

		if (ta == 0 && commandIndex != 0) {	//Cheap way of figuring out if String.toFloat failed
			#ifndef NODEBUG
			Serial.println("This command has stopped the routine.");
			#endif
			return -1;	//Tell loop() that pumping has completed
		}

		while (1) {	//Wait for the beginning of the gradient to start
			if (millis() - offsetTime >= ta * 1000) {	//If the queued event is up or has passed
				break;
			}
			activePumpingLoop();	//While we wait, do all the things.
			if(db(BUTTONSEL)){
				return -1;	//If the button is pushed, we tell loop() to go to jog mode
			}
		}

		unsigned long startTime = offsetTime + (ta * 1000);	//Start of the gradient where it SHOULD be
		while (1) {	//Loop for the duration of the gradient command

			if (recalculationInterval.trigger()) {
				float newQ = (float) mapFloat((millis() - startTime), 0.0, (tb - ta) * 1000.0, qa, qb);
				setQ(newQ);
			}

			if ((millis() - startTime) > (tb - ta) * 1000) {	//If the gradient is complete
				break;
			}
			activePumpingLoop();	//While we wait, do all the things.
			if(db(BUTTONSEL)){
				return -1;	//If the button is pushed, we tell loop() to go to jog mode
			}
		}
	} else {
		float t;
		float q;

		#ifndef NODEBUG
		Serial.println("Begin: NORMAL command parsing");
		Serial.print("Command: ");
		Serial.println(dataLine);
		#endif

		t = atof(strchr(dataLine, flagT) + 1);
		q = atof(strchr(dataLine, flagQ) + 1);

		#ifndef NODEBUG
		Serial.print("Command index: ");
		Serial.println(commandIndex);
		Serial.println(t);
		Serial.println(q);
		#endif

		if (t == 0 && commandIndex != 0) {	//Cheap way of figuring out if String.toFloat failed
			#ifndef NODEBUG
			Serial.println("This command has stopped the routine.");
			#endif
			return -1;	//Tell loop() that pumping has completed
		}

		while (1) {	//Wait for the event to be up
			if (millis() - offsetTime >= t * 1000) {	//If the queued event is up or has passed
				break;
			}
			activePumpingLoop();	//While we wait, do all the things.
			if(db(BUTTONSEL)){
				return -1;	//If the button is pushed, we tell loop() to go to jog mode
			}
		}
		setQ(q);
	}

	commandIndex++;
	return 0;	//Tell loop() that we're not finished pumping
}
