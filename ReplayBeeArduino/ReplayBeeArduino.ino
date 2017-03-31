#include <strobbio.h>

const int BUZZER_PIN = 9;
const int NOTE_GAP = 50; 
const int TOTAL_LIVES = 3;
const int LIVES_LED = 13;
const int GAME_ENDED = 0;
const int GAME_STARTED = 1;
const int GAME_ONGOING = 2;

int freqs[] = {131, 147, 165, 175, 196, 220, 247, 262, 294, 330, 349, 392, 440, 494, 523, 587, 659, 698, 784, 880, 988};
char notes[] = "1234567cdefgabCDEFGAB";
char melody[] = "     ";
char initMelody[] = "eeec_";
char correctMelody[] = "BB";
char wrongMelody[] = "1";
char winMelody[] = "e e f g g f e d c c d e e  dd_  e e f g g f e d c c d e d  cc_";
char looseMelody[] = "6  6 66 3c 77 66 66_";
int gameStatus = GAME_ENDED;
int levelIndex = 0;
int levels[][2] = {{2,500}, {3,400}, {4,300}, {5,300}, {5,200}};
int lives = TOTAL_LIVES;
StrobbioSettings strobbioSettings = StrobbioSettings();
StrobbioFrame frame;
Strobbio strobbio;

void setup() {
  Serial.begin(9600);

  strobbioSettings.frameLength = 16;
  strobbioSettings.oneInterval = 300;
  strobbioSettings.zeroInterval = 120;
  strobbioSettings.pulseInterval = 120;
  strobbio = Strobbio(strobbioSettings);

  pinMode(LIVES_LED, OUTPUT);

  randomSeed(analogRead(1));

  delay(1500);
  playMelody(initMelody, 80);
  Serial.println("Ready");    
}

void loop() {  
  char answer[] = "     ";
  getAnswer(answer);
  Serial.print("received: "); Serial.println(answer);

  //check for new game command
  if(strcmp(answer,"00000")==0) {newGame();} 
  
  if(gameStatus!=GAME_ENDED) {
    boolean newMelody=false;
    
    if(strcmp(answer,"00000")!=0) {
      //check last answer
      if(strcmp(answer,melody)==0) {
        levelIndex++;
        if(levelIndex>=5) {
          playMelody(winMelody, 130);
          gameStatus = GAME_ENDED;
          return;
        } else {
          confirmAction();
          newMelody = true;          
        }
      } else {        
        lives--;
        if(lives<=0) {
          playMelody(looseMelody, 300);
          gameStatus = GAME_ENDED;
          return;
        } else {
          playMelody(wrongMelody, 450);
        }        
      }
    }
    
    int totalNotes = levels[levelIndex][0];
    int noteDuration = levels[levelIndex][1];
    if((levelIndex==0 && gameStatus==GAME_STARTED) || newMelody) {createMelody(melody, totalNotes);}
    Serial.print("Level: "); Serial.print(levelIndex);
    Serial.print(" Melody: ");Serial.println(melody);
    delay(2500);
    playMelody(melody, noteDuration);
    
    gameStatus = GAME_ONGOING;
  }

}

void newGame() {
  confirmAction();
  levelIndex = 0;  
  lives = TOTAL_LIVES;
  gameStatus = GAME_STARTED;
}

void getAnswer(char* answer) {
  //block until data arrive
  int iterationCounter = 0;
  int livesIndicatorStatus = 0;
  while(strobbio.getStatus()!=STATUS_DATA) {
    if(gameStatus!=GAME_ENDED) {
      if((iterationCounter++)%10000==0) {
        if(livesIndicatorStatus<2*lives) {
          if(livesIndicatorStatus%2==0) {digitalWrite(LIVES_LED, HIGH);}
          else {digitalWrite(LIVES_LED, LOW);}
        } else if(livesIndicatorStatus>5*lives) {
          livesIndicatorStatus = -1;
        }
        livesIndicatorStatus++;
      }
    } else {
      digitalWrite(LIVES_LED, LOW);
    }
  }
  strobbio.getData(&frame);
  frame.print();
  
  for(int i=0; i<5; i++) {
    int value = frame.read(i*3,3);
    answer[i] = value>0?notes[6+value]:'0';
  }
  answer[5] = 0;  
}

boolean checkAnswer(char* melody, char* answer) {
  int i = 0;
  int melodyLength = strlen(melody);
  int answerLength = strlen(answer);
  if(answerLength<melodyLength) {return false;}
  for(i=0; i<melodyLength; i++) {
    Serial.print("comparing: "); Serial.print(melody[i]); Serial.print(" "); Serial.println(answer[i]);
    if(melody[i]!=answer[i]) {
      return false;
    }
  }
  for(i+1; i<answerLength; i++) {
    if(answer[i]!='7') {return false;}
  }
  return true;
}

void playNote(char note, int duration) {
  //get the note frequency
  int noteIndex = -1;
  for(int i=0; i<strlen(notes); i++) {
    if(note==notes[i]) {
      noteIndex = i;
      break;
    }
  }

  if(noteIndex>=0) {
    tone(BUZZER_PIN, freqs[noteIndex], duration);
  }
  delay(duration+NOTE_GAP);
}

void playMelody(char melody[], int duration) {
  for(int i=0; i<strlen(melody); i++) {
    int currentNote = melody[i];
    int currentDuration = duration;
    if(i+1<strlen(melody) && melody[i+1]=='_') {
      currentDuration = 2*duration;
      i++;
    }
    playNote(currentNote, currentDuration);
  }
}

void createMelody(char* melody, int length) {
  length = min(length, strlen(melody));
  for(int i=0; i<length; i++) {
    melody[i] = notes[7+random(7)];
  }
  for(int i=length; i<strlen(melody); i++) {
  melody[i] = '0';
  }
}

void confirmAction() {
  playMelody(correctMelody, 100);  
}

