// -*- mode: c++ -*-

// Detects whether the person is awake or not. It uses a crude motion detection algorithm.
class PirSleepDetection {

private:

	// PIR pin
	int pirPin;

	// Number of large movements that if occurred in a 5 minute period indicating the  person is awake.
	int awakeMovementThreshold;

  // Number of milliseconds the movement has to occur to indicate that it was an awake movement.
  int movementThresholdInMs;

	// Number of milliseconds to monitor before considering if the person is awake or not.
	unsigned long monitoringThresholdInMs;

	// Indicates we are monitoring
	bool isMonitoring;

	// Timestamp when monitoring started
	unsigned long monitoringStartTs;

	// Number of awake movements
	int numAwakeMovements;

	// Indicates if currently moving
	bool isCurrentlyMoving;

	// Timestamp when a movement started
	unsigned long movementStartTs;

public:

	// Constructor
	// threshold: number of movements indicating that the person is awake
	// monitoringThresholdInMins: number of minutes to monitor before considering the user is awake
 // movementThresholdInSec:number of seconds of movement to indicate awake movement
	PirSleepDetection(int movementThreshold=15, int monitoringThresholdInMins=5, int movementThresholdInSec=5) {
		awakeMovementThreshold = movementThreshold;
		monitoringThresholdInMs = monitoringThresholdInMins * 60L * 1000L;
    movementThresholdInMs = movementThresholdInSec * 1000L;
    isMonitoring = false;
	}

	// Attach the pin
	void attach(int pin) {
		pirPin = pin;
	}

	// Start monitoring movements
	void startMonitoring() {
		numAwakeMovements = 0;
		movementStartTs = 0;
		isCurrentlyMoving = false;
		isMonitoring = true;
		monitoringStartTs = millis();
	}

	// Stop monitoring movements
	void stopMonitoring() {
		isMonitoring = false;
	}

	// Update the detection state
	void update() {

		// If we are not montoring, do nothing
		if (!isMonitoring) {
			return;
		}

		readPirAndUpdateInternalState();
	}

	// Return whether if pir thinks the person is awake or not.
	bool isAwake() {

		// If not monitoring, return awake
		if (!isMonitoring) {
			return true;
		}

		// If still needs time to monitor, return awake
   unsigned long monitoringDuration = millis() - monitoringStartTs;
   unsigned long i = monitoringDuration % 1000;
   if (i > 900 || i < 100) {
    Serial.print("monitoring time: ");
    Serial.println(monitoringDuration);
   }
		if (monitoringDuration < monitoringThresholdInMs) {
			return true;
		}

    // If currently in a movement, let's wait until the person isn't moving before
    // indicating they are not awake.
    if (isCurrentlyMoving) {
      return true;
    }
    
		// Enough time has passed. Determine if the person is awake or not.
   Serial.print("number of awake movements: ");
   Serial.println(numAwakeMovements);
		return numAwakeMovements >= awakeMovementThreshold;
	}

private:

	// Update the internal state based on the pir state
	void readPirAndUpdateInternalState() {

		// Read current pir state
		int pirState = digitalRead(pirPin);

		// If currently moving and still moving...
		if (isCurrentlyMoving && pirState == HIGH) {
			// do nothing
		}

		// If currently moving and stopped moving...
		else if (isCurrentlyMoving && pirState == LOW) {
      Serial.println("detected end of movement");

			isCurrentlyMoving = false;

			// Only consider awake movements
      unsigned long movementDuration = millis() - movementStartTs;
      if (movementDuration >= movementThresholdInMs) {
				Serial.print("detected awake movement: ");
				numAwakeMovements++;
			} else {
        Serial.print("detected asleep movement: ");
			}
     Serial.println(movementDuration);
		}

		// If wasn't moving and is now moving...
		else if (pirState == HIGH) {
      Serial.println("detected start of movement");
			isCurrentlyMoving = true;
			movementStartTs = millis();
		}

		// If wasn't moving and still not moving...
		else {
			// do nothing
		}
	}
};
