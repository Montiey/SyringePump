void doJog() {
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
}

void getLine() {
	bool addNewline = false;
	if(!dataFile.available()) {
		#ifdef DEBUGSERIAL
		Serial.println("There is no remaining text in the file. Appending a possibly redundant newline.");
		#endif
		addNewline = true;
		endGame();
	}

	char c = ' ';    //some irrelevant starting value
	byte i = 0; //counter

	char x;
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
		#ifdef DEBUGSERIAL
		Serial.print("Line so far: ");
		Serial.println(dataLine);
		#endif
		i++;
	}
	#ifdef DEBUGSERIAL
	Serial.print("[getLine()] Line content: ");
	Serial.println(dataLine);
	#endif
}

void endGame() {
	#ifndef NOSERIAL
	Serial.println("\n\n-- Waiting for button --\n\n");
	#endif
	commandIndex = 0;
	dataFile.close();   //re-open the file to reset read() index
	dataFile = SD.open(CMDFILE);
	while(!digitalRead(BUTTONSEL)); //hold while pressed
	delay(100);
	while(digitalRead(BUTTONSEL));
	offsetTime = millis();
}

void doOpenLoopStuff() { //everything that needs to be done as often as possible
	digitalWrite(LEDG, HIGH);
	s.runSpeed();
	if(!digitalRead(BUTTONSEL)) {   //if the button is pushed, cancel the routine.
		digitalWrite(LEDG, LOW);
		endGame();
	}
	digitalWrite(LEDG, LOW);
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

void setMode(bool p0, bool p1, bool p2) {
	digitalWrite(MODE0, p0);
	digitalWrite(MODE1, p1);
	digitalWrite(MODE2, p2);
}

void everything(){
	#ifdef DEBUGSERIAL
	Serial.println("================= Loop is now running. Getting next command: ");
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
			endGame();
			return;
		}

		while (1) {
			if (millis() - offsetTime >= ta * 1000) { //if the queued event is up or has passed
				break;
			}
			doOpenLoopStuff();  //While we wait, do all the things.
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
			doOpenLoopStuff();  //While we wait, do all the things.
		}
	} else {
		#ifdef DEBUGSERIAL
		Serial.println("Begin: NORMAL command parsing");
		Serial.print("Command: ");
		Serial.println(dataLine);
		#endif
		float t;
		float q;

		//Get data
		#ifdef DEBUGSERIAL
		Serial.print("Data: ");
		Serial.println(strchr(dataLine, flagT) + 1);
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
			endGame();
			return;
		}

		while (1) {
			if (millis() - offsetTime >= t * 1000) { //if the queued event is up or has passed
				break;
			}
			doOpenLoopStuff();    //While we wait, do all the things.
		}
		setQ(q);
	}

	commandIndex++;    //target the next line and never look back
}
