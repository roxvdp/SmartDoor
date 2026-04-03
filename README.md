# Smart Door
Een smart‑lock project gebaseerd op een ESP32‑C6 met servo‑sluiting, keypad, LCD‑scherm, alarmsysteem en Blynk‑app‑integratie.

#### Video documentatie: https://youtu.be/zpoLW8mmaNo

## Inhoud
- Overzicht
- Hardware
- Features
- Software-architectuur
- Blynk-integratie
- Toekomstige-uitbreidingen

## Overzicht
De Smart Door is een slim deursysteem dat gebruik maakt van een ESP32‑C6. De deur wordt beveiligd met een servo‑slot, pincode‑invoer via knoppen, LCD‑statusdisplay, alarm bij te veel foutieve pogingen en Blynk‑app monitoring.

#### Het systeem kan:
- Deur openen/sluiten via servo
- Fysieke status detecteren via hallsensor
- Alarm activeren bij 3 foutieve codes
- Deur openen via touch‑sensor (binnenkant)
- Informatie tonen op LCD
- Status rapporteren aan Blynk

## Hardware
| Component       | Functie                          | Pin(s)          |
|----------------|----------------------------------|-----------------|
| Servo motor     | Deur vergrendelen/ontgrendelen   | GPIO 18         |
| Groene LED      | Toont ontgrendeld                | GPIO 15         |
| Rode LED        | Toont vergrendeld/alarm          | GPIO 21         |
| Buzzer          | Geluidsmeldingen                 | GPIO 22         |
| Knoppen         | Code‑invoer                      | GPIO 6, 7, 5, 17 |
| Hall‑sensor     | Detecteert gesloten deur         | GPIO 16         |
| Touch‑sensor    | Opent deur van binnen            | GPIO 23         |
| LCD (I2C)       | Status display                   | SDA = 3, SCL = 2 |

## Features
- Servo‑gestuurde deurvergrendeling
- Keypad met 4 knoppen (code: 1234)
- Alarm na 3 foutieve pogingen
- Touch‑sensor om deur van binnen te openen
- Automatisch sluiten na 5 seconden
- Waarschuwing bij >10s open deur
- LCD‑weergave van status

#### Blynk realtime monitoring:

| Virtual Pin | Betekenis |
|------------|-----------|
| V0         | Deurstatus (LOCK/UNLOCK) |
| V1         | Fysieke deurstatus (OPEN/CLOSE) |
| V2         | Alarm actief (1/0) |
| V3         | Foutieve pogingen |


## Software Architectuur
### Code‑invoer
- Elke druk op een knop voegt een cijfer toe aan inputCode.
- Bij 4 cijfers → checkCode().
- Bij 3 foutpogingen → activateAlarm().

### Hallsensor logica
Elke 200ms:
- Magnetisch veld aanwezig → deur dicht
- Sluit en lock als de deur te lang open staat.
- Geen magneet → deur open
- Start timer.
- Na 10 seconden waarschuwing.

### Alarm systeem
Wordt geactiveerd na 3 foute pogingen.
Signalen:
- Rode LED knippert
- Buzzer piept
- LCD toont “Alarm: ON”
- Reset door BTN1 + BTN2 tegelijk ingedrukt.

## Blynk Integratie
#### De ESP stuurt statusvariabelen naar Blynk:
- Blynk.virtualWrite(V0, doorIsLocked ? "LOCK" : "UNLOCK");
- Blynk.virtualWrite(V1, doorIsOpen ? "OPEN" : "CLOSE");
- Blynk.virtualWrite(V2, alarmActive ? 1 : 0);
- Blynk.virtualWrite(V3, failedAttempts);

#### Benodigd:
- Template ID
- Device Name
- Auth Token
- Wifi-gegevens worden in ssid en pass gezet.

## Toekomstige uitbreidingen
- Bewegingssensor voor slimme waarschuwingen
- Foto maken bij foutieve poging (ESP32‑CAM)
- Tweede Blynk‑controle: deur op afstand openen
- NFC of RFID toegang
- Logging via SD‑kaart
