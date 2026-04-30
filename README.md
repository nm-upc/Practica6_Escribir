# 🏷️ LECTOR/ESCRIPTOR RFID (ESP32)

Aquest projecte implementa un sistema per llegir i escriure text en targetes o clauers RFID (MIFARE) utilitzant un mòdul MFRC522 i un ESP32, amb interacció mitjançant el Monitor Sèrie.

Permet:
- Escriure text personalitzat (fins a 16 caràcters) en un bloc de la targeta.
- Llegir el text prèviament emmagatzemat.
- Veure l'identificador únic (UID) de la targeta detectada.

Tot es controla a través d'un menú interactiu per la consola sèrie.

---

## ⚙️ Connexions SPI

Configura els pins SPI de la següent manera per al mòdul MFRC522:

| Senyal MFRC522 | Pin ESP32 |
|----------------|-----------|
| RST (Reset)    | 7         |
| SDA / SS       | 39        |
| MOSI           | 35        |
| MISO           | 37        |
| SCK            | 36        |

*(També s'utilitzen els pins 3.3V i GND corresponents).*

---

## 🚀 Inicialització

Quan s'inicia el dispositiu:
1. Es configura la comunicació sèrie a 115200 bauds.
2. Es realitza un reset manual del mòdul RFID pel pin de RST.
3. S'inicialitza el bus SPI (a 1 MHz per estabilitat) i el mòdul MFRC522.
4. Es mostra la versió de firmware del lector per confirmar la connexió.
5. Es presenta el menú principal d'operacions.

---

## 💻 Funcionament i Comandes

El sistema utilitza una màquina d'estats (`MENU`, `ESCRIVINT`, `LLEGINT`) per gestionar les accions.

Al Monitor Sèrie, apareixerà el següent menú:
```text
=================================
   RFID - ESCRIURE / LLEGIR
=================================
Escriu una opcio:
  E - Escriure text a la targeta
  L - Llegir text de la targeta
=================================
>
```

### ✍️ Escriure a la targeta
1. Escriu la lletra `E` i prem Intro.
2. El sistema et demanarà el text (màxim 16 caràcters).
3. Escriu el text (ex: `Hola Món`) i prem Intro.
4. Acosta la teva targeta RFID al lector.
5. El sistema autenticarà, escriurà el text al bloc 1 i et confirmarà l'èxit.

### 📖 Llegir de la targeta
1. Escriu la lletra `L` i prem Intro.
2. Acosta la teva targeta RFID al lector.
3. El sistema llegirà el bloc 1 i mostrarà el text amagat.

---

## 🧠 Funcionament Tècnic

- **Neteja d'entrada**: S'inclou la funció `netejarEntrada()` per evitar problemes amb caràcters residuals del Monitor Sèrie (com `\r` o `\n`), ignorant els caràcters no imprimibles.
- **Autenticació**: Utilitza la clau de fàbrica per defecte (`0xFF` en tots els bytes) per accedir al bloc.
- **Bloc per defecte**: Totes les operacions es realitzen sobre el bloc número 1 (configurable en el codi mitjançant la variable `blocActual`).
- **Buffer de lectura segur**: S'utilitza un buffer de 18 bytes a la funció de lectura per acomodar els 16 bytes de dades més els 2 bytes de CRC afegits per la llibreria.

---

## 🛠️ Requisits

- ESP32
- Lector RFID MFRC522 i targetes/clauers compatibles (MIFARE Classic 1K).
- Llibreries:
  - `SPI.h` (Nativa)
  - `MFRC522.h` (S'ha d'instal·lar des del Gestor de Llibreries de l'IDE).

---

## ⚠️ Notes Importants

- L'espai màxim per operació d'escriptura està limitat a la grandària d'un bloc de dades MIFARE (16 bytes o caràcters).
- Si la lectura falla per error de CRC o autenticació, el mòdul es reinicia automàticament per evitar bloquejos.
- Recorda configurar el Monitor Sèrie a "Ambdós NL i CR" per a una entrada correcta de comandes.

---

## 👨‍💻 Autor
Projecte base per a l'estudi d'operacions bàsiques de lectura i escriptura de dades d'usuari en targetes RFID/NFC amb l'ecosistema ESP32.
