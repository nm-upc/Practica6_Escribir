#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN  7
#define SS_PIN   39

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

enum Estat { MENU, ESCRIVINT, LLEGINT };
Estat estatActual = MENU;

String textPerEscriure = "";
int blocActual = 1;

void mostrarMenu() {
  Serial.println("\n=================================");
  Serial.println("   RFID - ESCRIURE / LLEGIR");
  Serial.println("=================================");
  Serial.println("Escriu una opcio:");
  Serial.println("  E - Escriure text a la targeta");
  Serial.println("  L - Llegir text de la targeta");
  Serial.println("=================================");
  Serial.print("> ");
  estatActual = MENU;
  textPerEscriure = "";
}

void mostrarUID() {
  Serial.print("📇 UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) Serial.print(":");
  }
  Serial.println();
}

void finalitzarTargeta() {
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(500);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Reset manual
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  delay(200);
  digitalWrite(7, HIGH);
  delay(200);

  SPI.begin(36, 37, 35, 39);
  SPI.setFrequency(1000000);
  mfrc522.PCD_Init();
  delay(500);

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  mfrc522.PCD_SetAntennaGain(0x07);

  // Diagnostic
  byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print("Versio firmware: 0x");
  Serial.println(version, HEX);

  mostrarMenu();
}

// FIX: neteja robusta de l'entrada (\r\n, \n, espais)
String netejarEntrada(String s) {
  s.trim();
  // Eliminar caracters de control residuals
  String resultat = "";
  for (int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c >= 32 && c < 127) resultat += c;
  }
  return resultat;
}


void escriureTargeta() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("\n🎫 Targeta detectada!");
  mostrarUID();

  byte dades[16];
  for (byte i = 0; i < 16; i++) {
    dades[i] = (i < textPerEscriure.length()) ? (byte)textPerEscriure[i] : ' ';
  }

  Serial.print("🔐 Autenticant... ");
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, blocActual, &key, &(mfrc522.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    Serial.print("❌ Error: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    finalitzarTargeta();
    mostrarMenu();
    return;
  }
  Serial.println("✅");

  Serial.print("✍️  Escrivint... ");
  status = mfrc522.MIFARE_Write(blocActual, dades, 16);

  if (status == MFRC522::STATUS_OK) {
    Serial.println("✅");
    Serial.println("=================================");
    Serial.println("✅ TEXT GUARDAT CORRECTAMENT!");
    Serial.print("📝 Text: "); Serial.println(textPerEscriure);
    Serial.print("📍 Bloc: "); Serial.println(blocActual);
    Serial.println("=================================");
  } else {
    Serial.print("❌ Error escriptura: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  finalitzarTargeta();
  mostrarMenu();
}

void llegirTargeta() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("\n🎫 Targeta detectada!");
  mostrarUID();

  Serial.print("🔐 Autenticant... ");
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, blocActual, &key, &(mfrc522.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    Serial.print("❌ Error: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    finalitzarTargeta();
    mostrarMenu();
    return;
  }
  Serial.println("✅");

  // FIX: buffer de 18 bytes (16 dades + 2 bytes CRC que afegeix la llibreria)
  byte buffer[18];
  byte mida = sizeof(buffer); // Ha de ser 18

  Serial.print("📖 Llegint... ");
  status = mfrc522.MIFARE_Read(blocActual, buffer, &mida);

  if (status == MFRC522::STATUS_OK) {
    Serial.println("✅");
    Serial.println("=================================");
    Serial.print("📝 Text llegit: \"");
    for (byte i = 0; i < 16; i++) {
      if (buffer[i] >= 32 && buffer[i] < 127) {
        Serial.print((char)buffer[i]);
      }
    }
    Serial.println("\"");
    Serial.print("📍 Bloc: "); Serial.println(blocActual);
    Serial.println("=================================");
  } else {
    Serial.print("❌ Error lectura: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    // FIX: reiniciar el modul si hi ha error CRC
    mfrc522.PCD_Init();
    delay(50);
  }

  finalitzarTargeta();
  mostrarMenu();
}



void loop() {

  if (Serial.available() > 0) {
    String entrada = Serial.readStringUntil('\n');
    entrada = netejarEntrada(entrada); // FIX: neteja robusta

    if (entrada.length() == 0) return; // Ignorar línies buides

    if (estatActual == MENU) {
      entrada.toUpperCase();

      if (entrada == "E") {
        Serial.println("\n📝 MODO ESCRIPTURA");
        Serial.println("Escriu el text (max 16 caracters) i prem Enter:");
        Serial.print("> ");
        estatActual = ESCRIVINT;

      } else if (entrada == "L") {
        Serial.println("\n📖 MODO LECTURA");
        Serial.println("Acosta la targeta al lector...");
        estatActual = LLEGINT;

      } else {
        Serial.print("Opcio no valida (rebut: '");
        Serial.print(entrada);
        Serial.println("'). Escriu E o L.");
        Serial.print("> ");
      }

    } else if (estatActual == ESCRIVINT) {
      if (entrada.length() > 16) {
        Serial.print("❌ El text te ");
        Serial.print(entrada.length());
        Serial.println(" caracters (max 16). Torna a intentar-ho:");
        Serial.print("> ");
        return;
      }
      textPerEscriure = entrada;
      Serial.print("\n✅ Text a guardar: \"");
      Serial.print(textPerEscriure);
      Serial.println("\"");
      Serial.println("🟡 Acosta la targeta al lector...");
    }
  }

  if (estatActual == ESCRIVINT && textPerEscriure.length() > 0) {
    escriureTargeta();
  }

  if (estatActual == LLEGINT) {
    llegirTargeta();
  }
}
