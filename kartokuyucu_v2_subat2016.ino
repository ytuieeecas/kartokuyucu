/*YTÜ IEEE Kart Okuyucu Sistemi 2. versiyonu(2014 - 2015 döneminde yapılan versiyon)donanımına ait gömülü yazılım(firmware) kodudur.*/

/*YTÜ IEEE RAS/CAS ekibi tarafından yazılmıştır. Arduino Uno kartında çalışmaktadır. Kodun güncellenme tarihi: SUBAT 2016 */

/* Subat 2016 versiyonunda yapılan güncellemeler:
      1 - MikroSD kart takılı olmadığındaki okumalara uyarı ekranı koyulup kartın yeniden okutulması isteği.
      2 - İsteğe bağlı olarak BUZZER'dan gelen seslerin kapatılması.
      3 - Her oturuma giren kişi sayısının görülmesi için oturum modunun sag alt kosesine sayac eklenmesi.
      4 - Sabit stringlerin ram de yaer kaplamaması için F(string) kullanılması.
      (Not: SD kart takılı olmadığındaki okumalarda ses kapalı bile olsa buzzer ses çıkartır. Bu yazılımsal olarak değiştirilebilir. )
*/

/*
  Gerekli ek kütüphaneler: RC522 için https://github.com/miguelbalboa/rfid
                           I2C'li LCD için https://github.com/marcoschwartz/LiquidCrystal_I2C

  Bu kütüphaneler ile Arduino IDE 1.6.7 de sorunsuz çalıştığı denendi.
*/

int freeRam() /* Bu satır ve bundan sonraki süslü parantez içindeki yazılarda amac arduino'da kaç byte ram kaldığını serial ekranda göstermektir*/ 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

int OturumSayaci; /*Her oturuma giren kisi sayisini kontrol eden sayac*/

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <avr/pgmspace.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

#define BUZZER_PIN 3
#define SD_CS_PIN 4
#define BUTON_PIN_1 5
#define BUTON_PIN_2 6
#define BUTON_PIN_3 7
#define RFID_RST_PIN 9
#define RFID_SDA_PIN 10
#define YUKARIOK 0
#define ASAGIOK 1
#define ENTER 2
#define CHECK 15

uint8_t yukariOk[8] = {0x04, 0xE, 0x15, 0x04, 0x04, 0x04, 0x04};
uint8_t asagiOk[8] = {0x04, 0x04, 0x04, 0x04, 0x15, 0xE, 0x04};
uint8_t check[8] = {0x0, 0x1, 0x3, 0x16, 0x1c, 0x8, 0x0};

File myFile;
LiquidCrystal_I2C lcd(0x27, 20, 4);
MFRC522 mfrc522(RFID_SDA_PIN, RFID_RST_PIN);
byte cardPresent;
byte uidByte[] = {0, 0, 0, 0};

//ŞİFRE DEĞŞİTİRİLMEK İSTENİYORSA asilSifre DİZİSİ DEĞİŞTİRİLMELİDİR
//Şifre, butonlara dizideki sırayla basılarak girilir
// 3 --> YUKARI TUŞU
// 2 --> ENTER TUŞU
// 1 --> ASAĞI TUŞU
const int asilSifre[] = {2, 1, 1, 3};//Şifre: enter, aşağı, aşağı, yukarı

/*Ana menü ve alt menülerde kaç seçeneğin olacağının tutulduğu değişkenler aşağıdadır. (Örn: Ana menüde "otorum modu", " kayıt modu" olarak 2 tane vb.)
  "+1" ler "Geri" seçeneği içindir. Alt menülerdeki sayılar yapılacak etkinlikteki oturum sayılarına vb. ne göre değiştirilmelidir.*/
const int secenekSayisiAnaMenu = 3;
const int secenekSayisiAltMenu1 = 3 + 1;
const int secenekSayisiAltMenu11 = 11 + 1;
const int secenekSayisiAltMenu12 = 11 + 1;
const int secenekSayisiAltMenu13 = 11 + 1;
const int secenekSayisiEvetHayir = 2;
const int secenekSayisiAltMenu3 = 2; // Ses aç-kapa menüsü için eklendi: şubat2016


/*Menü ve menü seçeneklerinin isimlerinin tutulduğu diziler aşağıdadır. Oturumların isimleri ve sayıları etkinliklere göre yukarıdaki
  seçenek sayısı değişikleriyle birlikte değiştirilebilir.*/
char* anaMenuStringler[] = {"OTURUM MODU", "YENI KAYIT MODU", "SES DURUMU"};
char* altMenu1Stringler[] = {"1.Gun", "2.Gun", "3.Gun"};

const char oturum1_01[] PROGMEM = "1.Oturum";
const char oturum1_02[] PROGMEM = "2.Oturum";
const char oturum1_03[] PROGMEM = "3.Oturum";
const char oturum1_04[] PROGMEM = "4.Oturum";
const char oturum1_05[] PROGMEM = "5.Oturum";
const char oturum1_06[] PROGMEM = "6.Oturum";
const char oturum1_07[] PROGMEM = "7.Oturum";
const char oturum1_08[] PROGMEM = "8.Oturum";
const char oturum1_09[] PROGMEM = "9.Oturum";
const char oturum1_10[] PROGMEM = "10.Oturum";
const char oturum1_11[] PROGMEM = "11.Oturum";

const char oturum2_01[] PROGMEM = "1.Oturum";
const char oturum2_02[] PROGMEM = "2.Oturum";
const char oturum2_03[] PROGMEM = "3.Oturum";
const char oturum2_04[] PROGMEM = "4.Oturum";
const char oturum2_05[] PROGMEM = "5.Oturum";
const char oturum2_06[] PROGMEM = "6.Oturum";
const char oturum2_07[] PROGMEM = "7.Oturum";
const char oturum2_08[] PROGMEM = "8.Oturum";
const char oturum2_09[] PROGMEM = "9.Oturum";
const char oturum2_10[] PROGMEM = "10.Oturum";
const char oturum2_11[] PROGMEM = "11.Oturum";

const char oturum3_01[] PROGMEM = "1.Oturum";
const char oturum3_02[] PROGMEM = "2.Oturum";
const char oturum3_03[] PROGMEM = "3.Oturum";
const char oturum3_04[] PROGMEM = "4.Oturum";
const char oturum3_05[] PROGMEM = "5.Oturum";
const char oturum3_06[] PROGMEM = "6.Oturum";
const char oturum3_07[] PROGMEM = "7.Oturum";
const char oturum3_08[] PROGMEM = "8.Oturum";
const char oturum3_09[] PROGMEM = "9.Oturum";
const char oturum3_10[] PROGMEM = "10.Oturum";
const char oturum3_11[] PROGMEM = "11.Oturum";


const char* const altMenu11Stringler[] PROGMEM = {oturum1_01, oturum1_02, oturum1_03, oturum1_04, oturum1_05, oturum1_06, oturum1_07, oturum1_08, oturum1_09, oturum1_10, oturum1_11};
const char* const altMenu12Stringler[] PROGMEM = {oturum2_01, oturum2_02, oturum2_03, oturum2_04, oturum2_05, oturum2_06, oturum2_07, oturum2_08, oturum2_09, oturum2_10, oturum2_11};
const char* const altMenu13Stringler[] PROGMEM = {oturum3_01, oturum3_02, oturum3_03, oturum3_04, oturum3_05, oturum3_06, oturum3_07, oturum3_08, oturum3_09, oturum3_10, oturum3_11};

char buffer[15];

int secenekler[] = {1, 0, 0, 0};

boolean sesDurumu = true;                              
boolean butonOncekiDurum[] = {false, false, false};
char rakamlar[] = "012345679";
char txtIsmi[] = "otrm    .txt";
int menuDegisimDegiskeni = 99;
int menuIciEkranNo = 0;
int girilecekSifre[] = {0, 0, 0, 0};
unsigned long butonBasmaAni = 0;
int secenekNumEvetHayir = 2;
int sdRfidGecisDegiskeni = 1;
boolean uidSifirlamaDurumu = false;

///////////////////////////////////////////////////////////////////////////////////////////////SETUP

void setup() {

  pinMode(BUTON_PIN_1, INPUT);
  pinMode(BUTON_PIN_2, INPUT);
  pinMode(BUTON_PIN_3, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(SD_CS_PIN, OUTPUT);
  pinMode(RFID_SDA_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);
  digitalWrite(RFID_SDA_PIN, HIGH);

  
  Serial.begin(9600);
//  Serial.println(freeRam());  // !!!Ram deki kullanılabilir byte sayısını ekrana yazar. Eğer boş ram miktarı debug edilmeyecekse, bu iki serial
//  Serial.println ();          // komutu yorum şeklinde bırakılmalıdır. Yoksa program ilk açıldığında gereksiz yere bilgisayara bu veriler gönderilir.


  SPI.begin();
  SD.begin(SD_CS_PIN);
  mfrc522.PCD_Init();
  lcd.init();
  lcd.createChar(YUKARIOK, yukariOk);
  lcd.createChar(ASAGIOK, asagiOk);
  lcd.createChar(CHECK, check);
  lcd.backlight();

}

///////////////////////////////////////////////////////////////////////////////////////////////ANA PROGRAM

void loop() {

  if (secenekler[0] > 0 && secenekler[1] == 0) { //ANA MENÜ

    EkranaYazAnaMenu();

    SecenekDegistir(secenekler[0], secenekSayisiAnaMenu);

    if (EnterTusu()) {
      SonrakiMenuFonk(secenekler[1]);
    }

  }

  if (secenekler[0] == 1 && secenekler[1] > 0 && secenekler[2] == 0) { //GÜNLER MENÜSÜ

    EkranaYazAltMenu1();

    SecenekDegistir(secenekler[1], secenekSayisiAltMenu1);

    if (EnterTusu()) {
      if (GeriSecenegiDurumu(secenekler[1], secenekSayisiAltMenu1) == true) {
        GeriSecenegiFonk(secenekler[1]);
      } else {
        SonrakiMenuFonk(secenekler[2]);
      }
    }

  }

  if (secenekler[0] == 1 && secenekler[1] == 1 && secenekler[2] > 0 && secenekler[3] == 0) { //1.GÜN OTURUMLAR MENÜSÜ

    EkranaYazAltMenu11();

    SecenekDegistir(secenekler[2], secenekSayisiAltMenu11);

    if (EnterTusu()) {
      if (GeriSecenegiDurumu(secenekler[2], secenekSayisiAltMenu11) == true) {
        GeriSecenegiFonk(secenekler[2]);
      } else {
        SonrakiMenuFonk(secenekler[3]);
      }
    }

  }

  if (secenekler[0] == 1 && secenekler[1] == 2 && secenekler[2] > 0 && secenekler[3] == 0) { //2.GÜN OTURUMLAR MENÜSÜ

    EkranaYazAltMenu12();

    SecenekDegistir(secenekler[2], secenekSayisiAltMenu12);

    if (EnterTusu()) {
      if (GeriSecenegiDurumu(secenekler[2], secenekSayisiAltMenu12) == true) {
        GeriSecenegiFonk(secenekler[2]);
      } else {
        SonrakiMenuFonk(secenekler[3]);
      }
    }

  }

  if (secenekler[0] == 1 && secenekler[1] == 3 && secenekler[2] > 0 && secenekler[3] == 0) { //3.GÜN OTURUMLAR MENÜSÜ

    EkranaYazAltMenu13();

    SecenekDegistir(secenekler[2], secenekSayisiAltMenu13);

    if (EnterTusu()) {
      if (GeriSecenegiDurumu(secenekler[2], secenekSayisiAltMenu13) == true) {
        GeriSecenegiFonk(secenekler[2]);
      } else {
        SonrakiMenuFonk(secenekler[3]);
      }
    }

  }

  if (secenekler[0] == 1 && secenekler[1] > 0 && secenekler[2] > 0 && secenekler[3] == 1) { //ŞİFRE EKRANI OTURUM MODU

    SifreEkrani();
    SifreGir(secenekler[3]);
  }

  if (secenekler[0] == 1 && secenekler[1] > 0 && secenekler[2] > 0 && secenekler[3] == 2) { //KAYIT EKRANI OTURUM MODU

    OturumKayitEkrani();
    if (ButonUzunBasmaDurumu(BUTON_PIN_2, 2, 1000)) {
      secenekler[3] = 3;
    }
    SdRfidAltProgram();

  }

  if (secenekler[0] == 1 && secenekler[1] > 0 && secenekler[2] > 0 && secenekler[3] == 3) { //KAYITTAN ÇIKIŞ EKRANI OTURUM MODU

    KartOkumaCikisEkrani();
    SecenekDegistir(secenekNumEvetHayir, secenekSayisiEvetHayir);

    if (EnterTusu()) {
      if (secenekNumEvetHayir == 1) {
        secenekler[3] = 0;
        uidSifirlamaDurumu = true;
      } else {
        secenekler[3] = 2;
      }
    }

  }

  if (secenekler[0] == 2 && secenekler[1] == 1) { //ŞİFRE EKRANI YENİ KAYIT MODU

    SifreEkrani();
    SifreGir(secenekler[1]);
  }

  if (secenekler[0] == 2 && secenekler[1] == 2) { //KAYIT EKRANI YENİ KAYIT MODU

    KatilimciKayitEkrani();

    if (ButonUzunBasmaDurumu(BUTON_PIN_2, 2, 1000)) {
      secenekler[1] = 3;
    }
    if (ButonUzunBasmaDurumu(BUTON_PIN_1, 1, 500) == HIGH) {
      UidByteSifirla();
      SonIdSilindiYazisi();
    }

    RfidToSerialAltProgram();

  }

  if (secenekler[0] == 2 && secenekler[1] == 3) { //KAYITTAN ÇIKIŞ EKRANI YENİ KAYIT MODU

    KartOkumaCikisEkrani();
    SecenekDegistir(secenekNumEvetHayir, secenekSayisiEvetHayir);

    if (EnterTusu()) {
      if (secenekNumEvetHayir == 1) {
        secenekler[1] = 0;
        uidSifirlamaDurumu = true;
      } else {
        secenekler[1] = 2;
      }
    }

  }

  if (secenekler[0] == 3 && secenekler[1] > 0) { //SES AÇ-KAPA AYARI

    if (sesDurumu == true) {
      EkranaYazAltMenu3a();
      SecenekDegistir(secenekler[1], secenekSayisiAltMenu3);
      if (EnterTusu()) {
        if (GeriSecenegiDurumu(secenekler[1], secenekSayisiAltMenu3) == true) {
          GeriSecenegiFonk(secenekler[1]);
        } else {
          sesDurumu = !sesDurumu;
          sesKapandiEkrani();
          GeriSecenegiFonk(secenekler[1]);//secenekler[1]=0;
        }
      }
    }
    else {
      EkranaYazAltMenu3b();
      SecenekDegistir(secenekler[1], secenekSayisiAltMenu3);
      if (EnterTusu()) {
        if (GeriSecenegiDurumu(secenekler[1], secenekSayisiAltMenu3) == true) {
          GeriSecenegiFonk(secenekler[1]);
        } else {
          sesDurumu = !sesDurumu;
          sesAcildiEkrani();
          GeriSecenegiFonk(secenekler[1]);                //secenekler[1]=0;
        }
      }
    }


  }



}

///////////////////////////////////////////////////////////////////////////////////////////////ALT PROGRAMLAR

///////////////////////////////////////////////////////////////////////////////////////////////Kart okuyucu işleviyle ilgili fonksiyonlar

void RfidToSerialAltProgram() {

  if (uidSifirlamaDurumu == true) {
    UidByteSifirla();
    uidSifirlamaDurumu = false;
  }

  digitalWrite(SD_CS_PIN, HIGH);
  delay(3);
  digitalWrite(RFID_SDA_PIN, LOW);

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
  }
  else {
    if ( ! mfrc522.PICC_ReadCardSerial()) {
    }
    else {
      if (UidKarsilastir() == true) {

        BuKartOkutulmusturYazisi(200);

      }
      while (UidKarsilastir() == false) {
        for (byte i = 0; i < 4; i++) {
          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
          Serial.print(mfrc522.uid.uidByte[i], HEX);
          uidByte[i] = mfrc522.uid.uidByte[i];
        }
        KartOkutulduEkrani();
        delay(300);
        KatilimciKayitEkrani();
      }
    }
  }
}

void SdRfidAltProgram() {

  if (uidSifirlamaDurumu == true) {
    UidByteSifirla();
    uidSifirlamaDurumu = false;
  }

  if (sdRfidGecisDegiskeni == 1) {

    digitalWrite(SD_CS_PIN, HIGH);
    delay(3);
    digitalWrite(RFID_SDA_PIN, LOW);

    if ( ! mfrc522.PICC_IsNewCardPresent()) {
    }
    else {
      if ( ! mfrc522.PICC_ReadCardSerial()) {
      }
      else {
        if (UidKarsilastir() == true) {

          BuKartOkutulmusturYazisi(200);

        }
        while (UidKarsilastir() == false) {
          for (byte i = 0; i < 4; i++) {
            uidByte[i] = mfrc522.uid.uidByte[i];
          }
          OturumSayaci = OturumSayaci + 1;
          sdRfidGecisDegiskeni = 2;
        }
      }
    }
  }

  while (sdRfidGecisDegiskeni == 2) {

    digitalWrite(RFID_SDA_PIN, HIGH);
    delay(3);
    digitalWrite(SD_CS_PIN, LOW);


    TxtIsmiOlustur(secenekler[1], secenekler[2]);
    myFile = SD.open(txtIsmi, FILE_WRITE);

    if (myFile) {

      for (byte i = 0; i < 4; i++) {
        myFile.print(uidByte[i] < 0x10 ? "0" : "");
        myFile.print(uidByte[i], HEX);
      }
      myFile.println();
      myFile.close();

    }  else {
      OturumSayaci = OturumSayaci - 1; /* sd kart takılı değilken sayacın artmaması için oluşturuldu */
      cildir();
    }

    KartOkutulduEkrani();
    if (sesDurumu == true) {
      digitalWrite(BUZZER_PIN, HIGH);
    }
    delay(75);
    digitalWrite(BUZZER_PIN, LOW);
    delay(40);
    if (sesDurumu == true) {
      digitalWrite(BUZZER_PIN, HIGH);
    }
    delay(75);
    digitalWrite(BUZZER_PIN, LOW);
    OturumKayitEkrani();

    delay(500);
    sdRfidGecisDegiskeni = 1;
  }

}

void TxtIsmiOlustur(int secenekNum1, int secenekNum2) {

  int onlarBasamagi = 0; int birlerBasamagi = 0;
  txtIsmi[4] = rakamlar[secenekNum1];
  txtIsmi[5] = '_';
  if (secenekNum2 < 10) {
    txtIsmi[6] = rakamlar[0];
    txtIsmi[7] = rakamlar[secenekNum2];
  } else {
    onlarBasamagi = secenekNum2 / 10; birlerBasamagi = secenekNum2 % 10;
    txtIsmi[6] = rakamlar[onlarBasamagi];
    txtIsmi[7] = rakamlar[birlerBasamagi];
  }

}

void UidByteSifirla() {

  uidByte[0] = 0;
  uidByte[1] = 0;
  uidByte[2] = 0;
  uidByte[3] = 0;

}


boolean UidKarsilastir() {

  boolean a = false;
  for (int i = 0; i < 4; i++) {
    if (uidByte[i] == mfrc522.uid.uidByte[i]) {
      a = true;
    }
    else {
      a = false;
      break;
    }

  }

  return a;
}

///////////////////////////////////////////////////////////////////////////////////////////////Menü işleyişi fonksiyonları

void SecenekDegistir(int &secenekNum, int secenekSayisi) {

  if (YukariTusu()) {
    secenekNum--;
    if (secenekNum < 1) {
      secenekNum = secenekSayisi;
    }
    ImlecYukari(secenekNum, secenekSayisi);
  }
  if (AsagiTusu()) {
    secenekNum++;
    if (secenekNum > secenekSayisi) {
      secenekNum = 1;
    }
    ImlecAsagi(secenekNum, secenekSayisi);
  }

}

boolean MenuDegisimDurumu() {

  if ((secenekler[2] > 0 && secenekler[3] == 3 || secenekler[0] == 2 && secenekler[1] == 3)) {
    if (menuDegisimDegiskeni == 5) {
      return false;
    } else {
      menuDegisimDegiskeni = 5;
      return true;
    }
  }
  else if ((secenekler[2] > 0 && secenekler[3] == 2)) {
    if (menuDegisimDegiskeni == 4) {
      return false;
    } else {
      menuDegisimDegiskeni = 4;
      return true;
    }
  }
  else if ((secenekler[2] > 0 && secenekler[3] == 1) || (secenekler[0] == 2 && secenekler[1] == 1)) {
    if (menuDegisimDegiskeni == 3) {
      return false;
    } else {
      menuDegisimDegiskeni = 3;
      return true;
    }
  }
  else {
    for (int i = 0; i < 3; i++) {

      if (secenekler[i] > 0 && secenekler[i + 1] == 0) {
        if (menuDegisimDegiskeni == i) {
          return false;
        }
        else {
          menuDegisimDegiskeni = i;
          return true;
        }
      }
    }
  }

}

boolean MenuIciEkranKaydirmaDurumu(int secenekNum) {

  int lol = 0;
  lol = (secenekNum - 1) / 4;
  if (lol != menuIciEkranNo) {
    return true;
  } else {
    return false;
  }

}

boolean GeriSecenegiDurumu(int secenekNum, int secenekSayisi) {
  if (secenekNum == secenekSayisi) {
    return true;
  } else {
    return false;
  }
}

void GeriSecenegiFonk(int &secenekNum) {
  secenekNum = 0;
}

void SonrakiMenuFonk(int &secenekNum) {
  secenekNum = 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////Şifre işleyişi fonksiyonları

void SifreGir(int &secenekNum) {
  if (ButonBasilmaDurumu() == true && SifreHaneBosMu(1) == true) {
    SifreHaneYaz(1);
  } else if (ButonBasilmaDurumu() == true && SifreHaneBosMu(2) == true) {
    SifreHaneYaz(2);
  } else if (ButonBasilmaDurumu() == true && SifreHaneBosMu(3) == true) {
    SifreHaneYaz(3);
  } else if (ButonBasilmaDurumu() == true && SifreHaneBosMu(4) == true) {
    SifreHaneYaz(4);
  } else if (SifreButunHanelerGirildiMi(4) == true) {
    if (SifreKontrol() == true) {
      delay(100);
      SifreDogruEkrani();
      secenekNum = 2;
      GirilenSifreyiTemizle();
    }
    else {
      delay(100);
      SifreYanlisEkrani();
      secenekNum = 0;
      GirilenSifreyiTemizle();
    }
  }
}

void GirilenSifreyiTemizle() {

  for (int i = 0; i < 4; i++) {
    girilecekSifre[i] = 0;
  }

}

boolean SifreKontrol() {

  for (int i = 0; i < 4; i++) {
    if (girilecekSifre[i] != asilSifre[i]) {
      return false;
    }
  }
  return true;

}

boolean SifreButunHanelerGirildiMi(int haneSayisi) {

  for (int i = 0; i < haneSayisi; i++) {
    if (girilecekSifre[i] == 0) {
      return false;
    }
  }
  return true;
}

boolean SifreHaneBosMu(int haneSira) {

  for (int i = 0; i < (haneSira - 1); i++) {
    if (girilecekSifre[i] == 0) {
      return false;
    }
  }
  if (girilecekSifre[haneSira - 1] == 0) {
    return true;
  } else {
    return false;
  }
}

void SifreHaneYaz(int haneSira) {

  if (YukariTusu()) {
    girilecekSifre[haneSira - 1] = 3;
    SifreHaneYildizKoy(haneSira);
  }
  else if (EnterTusu()) {
    girilecekSifre[haneSira - 1] = 2;
    SifreHaneYildizKoy(haneSira);
  }
  else if (AsagiTusu()) {
    girilecekSifre[haneSira - 1] = 1;
    SifreHaneYildizKoy(haneSira);
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////Buton fonksiyonları

boolean ButonBasilmaDurumu() {

  if (digitalRead(BUTON_PIN_1) == HIGH || digitalRead(BUTON_PIN_2) == HIGH || digitalRead(BUTON_PIN_3) == HIGH) {
    return true;
  } else {
    return false;
  }

}

boolean ButonUzunBasmaDurumu(int butonPin, int butonNo, unsigned long sure) {

  if (ButonCikanKenar(butonPin, butonNo) == true) {
    butonBasmaAni = millis();
  }
  if (digitalRead(butonPin) == HIGH) {
    if (millis() - butonBasmaAni > sure) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }

}

boolean YukariTusu() {

  if (ButonCikanKenar(BUTON_PIN_3, 3) == true) {
    return true;
  }
  else {
    return false;
  }

}

boolean EnterTusu() {

  if (ButonCikanKenar(BUTON_PIN_2, 2) == true) {
    return true;
  }
  else {
    return false;
  }

}

boolean AsagiTusu() {

  if (ButonCikanKenar(BUTON_PIN_1, 1) == true) {
    return true;
  }
  else {
    return false;
  }

}

boolean DeBounceluButonDurum(int a) {

  if (digitalRead(a) == true) {
    delay(30);
    if (digitalRead(a) == true) {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }

}


boolean ButonCikanKenar(int butonPin, int butonNo) {

  if (butonOncekiDurum[butonNo - 1] == false) {

    if (DeBounceluButonDurum(butonPin) == true) {
      butonOncekiDurum[butonNo - 1] = true;
      return true;
    }
    else {
      butonOncekiDurum[butonNo - 1] = false;
      return false;
    }
  }
  else {
    if (DeBounceluButonDurum(butonPin) == true) {
      butonOncekiDurum[butonNo - 1] = true;
      return false;
    }
    else {
      butonOncekiDurum[butonNo - 1] = false;
      return false;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////Ekrana bastırma fonksiyonları

void ImlecYukari(int secenekNum, int secenekSayisi) {

  if (secenekNum == secenekSayisi) {
    lcd.setCursor(0, 0);
    lcd.print("   ");
  } else {
    lcd.setCursor(0, ((secenekNum + 1) - 1) % 4);
    lcd.print("   ");
  }

  lcd.setCursor(0, (secenekNum - 1) % 4);
  lcd.print("-> ");

}

void ImlecAsagi(int secenekNum, int secenekSayisi) {

  if (secenekNum == 1) {
    lcd.setCursor(0, (secenekSayisi - 1) % 4);
    lcd.print("   ");
  } else {
    lcd.setCursor(0, ((secenekNum - 1) - 1) % 4);
    lcd.print("   ");
  }

  lcd.setCursor(0, (secenekNum - 1) % 4);
  lcd.print("-> ");

}

void ImlecIlkYer(int secenekNum) {

  lcd.setCursor(0, (secenekNum - 1) % 4);
  lcd.print("-> ");

}

void EkranaYazAnaMenu() {

  if (MenuDegisimDurumu() == true) {

    lcd.clear();
    ImlecIlkYer(secenekler[0]);
    lcd.setCursor(3, 0);
    lcd.print(anaMenuStringler[0]);
    lcd.setCursor(3, 1);
    lcd.print(anaMenuStringler[1]);
    lcd.setCursor(3, 2);
    lcd.print(anaMenuStringler[2]);

  }
}

void EkranaYazAltMenu1() {

  if (MenuDegisimDurumu() == true) {

    lcd.clear();
    ImlecIlkYer(secenekler[1]);
    lcd.setCursor(3, 0);
    lcd.print(altMenu1Stringler[0]);
    lcd.setCursor(3, 1);
    lcd.print(altMenu1Stringler[1]);
    lcd.setCursor(3, 2);
    lcd.print(altMenu1Stringler[2]);
    lcd.setCursor(3, 3);
    lcd.print(F("GERI"));

  }
}

void EkranaYazAltMenu11() {

  if (MenuIciEkranKaydirmaDurumu(secenekler[2]) == true || MenuDegisimDurumu() == true) {
    int eklenecekNo = 0;
    menuIciEkranNo = (secenekler[2] - 1) / 4; eklenecekNo = menuIciEkranNo * 4;

    lcd.clear();
    ImlecIlkYer(secenekler[2]);
    for (int i = 0; i < 4; i++) {
      if (eklenecekNo + i > (secenekSayisiAltMenu11 - 2)) {
        lcd.setCursor(3, i);
        lcd.print(F("GERI"));
        break;
      }
      lcd.setCursor(3, i);
      strcpy_P(buffer, (char*)pgm_read_word(&(altMenu11Stringler[eklenecekNo + i])));
      lcd.print(buffer);
    }

  }
}

void EkranaYazAltMenu12() {

  if (MenuIciEkranKaydirmaDurumu(secenekler[2]) == true || MenuDegisimDurumu() == true) {
    int eklenecekNo = 0;
    menuIciEkranNo = (secenekler[2] - 1) / 4; eklenecekNo = menuIciEkranNo * 4;

    lcd.clear();
    ImlecIlkYer(secenekler[2]);
    for (int i = 0; i < 4; i++) {
      if (eklenecekNo + i > (secenekSayisiAltMenu12 - 2)) {
        lcd.setCursor(3, i);
        lcd.print(F("GERI"));
        break;
      }
      lcd.setCursor(3, i);
      strcpy_P(buffer, (char*)pgm_read_word(&(altMenu12Stringler[eklenecekNo + i])));
      lcd.print(buffer);
    }

  }
}

void EkranaYazAltMenu13() {

  if (MenuIciEkranKaydirmaDurumu(secenekler[2]) == true || MenuDegisimDurumu() == true) {
    int eklenecekNo = 0;
    menuIciEkranNo = (secenekler[2] - 1) / 4; eklenecekNo = menuIciEkranNo * 4;

    lcd.clear();
    ImlecIlkYer(secenekler[2]);
    for (int i = 0; i < 4; i++) {
      if (eklenecekNo + i > (secenekSayisiAltMenu13 - 2)) {
        lcd.setCursor(3, i);
        lcd.print(F("GERI"));
        break;
      }
      lcd.setCursor(3, i);
      strcpy_P(buffer, (char*)pgm_read_word(&(altMenu13Stringler[eklenecekNo + i])));
      lcd.print(buffer);
    }

  }
}

void SifreEkrani() {

  if (MenuDegisimDurumu() == true) {

    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print(F("SIFREYI GIRINIZ"));
    lcd.setCursor(6, 2);
    lcd.print("_ _ _ _");

  }

}

void SifreDogruEkrani() {

  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print(F("DOGRU SIFRE"));
  delay(500);

}

void SifreYanlisEkrani() {

  if (sesDurumu == true) {
    digitalWrite(BUZZER_PIN, HIGH);
  }
  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print(F("YANLIS SIFRE"));
  delay(300);
  lcd.noBacklight();
  delay(150);
  lcd.backlight();
  delay(300);
  lcd.noBacklight();
  delay(150);
  lcd.backlight();
  digitalWrite(BUZZER_PIN, LOW);

}

void SifreHaneYildizKoy(int haneSira) {

  lcd.setCursor((5 + (haneSira * 2) - 1), 2);
  lcd.print("*");

}

void KatilimciKayitEkrani() {

  if (MenuDegisimDurumu() == true) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("-KARTINIZI OKUTUNUZ-"));
    lcd.setCursor(0, 2);
    lcd.print(F("Son idyi silmek icin"));
    lcd.setCursor(0, 3);
    lcd.printByte(ASAGIOK); lcd.print(" "); lcd.print("tusuna uzun basin");
  }

}

void OturumKayitEkrani() {

  if (MenuDegisimDurumu() == true) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("-KARTINIZI OKUTUNUZ-"));
    lcd.setCursor(0, 2);
    lcd.print(F("Kayitlar aliniyor:"));
    lcd.setCursor(0, 3);
    lcd.print(altMenu1Stringler[secenekler[1] - 1]);
    lcd.print(" ");
   
    if (secenekler[1] == 1) {
      strcpy_P(buffer, (char*)pgm_read_word(&(altMenu11Stringler[secenekler[2] - 1])));
      lcd.print(buffer);
       lcd.setCursor(17,3);
    lcd.print(OturumSayaci);
      } else if (secenekler[1] == 2) {
   
      strcpy_P(buffer, (char*)pgm_read_word(&(altMenu12Stringler[secenekler[2] - 1])));
      lcd.print(buffer);
       lcd.setCursor(17,3);
    lcd.print(OturumSayaci);
    } else if (secenekler[1] == 3) {
     
      strcpy_P(buffer, (char*)pgm_read_word(&(altMenu13Stringler[secenekler[2] - 1])));
      lcd.print(buffer);
    lcd.setCursor(17,3);
    lcd.print(OturumSayaci);
    }
  }

}

void KartOkumaCikisEkrani() {

  if (MenuDegisimDurumu() == true) {

    lcd.clear();
    secenekNumEvetHayir = 2; ImlecIlkYer(secenekNumEvetHayir);
    lcd.setCursor(3, 0);
    lcd.print(F("EVET"));
    lcd.setCursor(3, 1);
    lcd.print(F("HAYIR"));
    if (secenekler[3] == 3 && secenekler[2] > 0) {
      lcd.setCursor(14, 1);
      lcd.print(F("Oturum"));
      lcd.setCursor(13, 2);
      lcd.print(F("secmeye"));
      lcd.setCursor(8, 3);
      lcd.print(F("donulsun mu?"));
    }
    if (secenekler[0] == 2 && secenekler[1] == 3) {
      lcd.setCursor(10, 1);
      lcd.print(F("Yeni kayit"));
      lcd.setCursor(12, 2);
      lcd.print(F("modundan"));
      lcd.setCursor(8, 3);
      lcd.print(F("cikilsin mi?"));
    }

  }
}

void BuKartOkutulmusturYazisi(int sure) {

  lcd.setCursor(0, 1);
  lcd.print(F("Bu kart okutulmustur"));
  delay(sure);
  lcd.setCursor(0, 1);
  lcd.print("                    ");
  delay(sure / 2);

}

void KartOkutulduEkrani() {

  menuDegisimDegiskeni = 100;
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print(F("<<KART OKUTULDU>>"));

}

void SonIdSilindiYazisi() {

  lcd.setCursor(3, 1);
  lcd.print(F("Son id silindi"));
  delay(200);
  lcd.setCursor(3, 1);
  lcd.print("              ");
  delay(100);
  lcd.setCursor(3, 1);
  lcd.print(F("Son id silindi"));
  delay(200);
  lcd.setCursor(3, 1);
  lcd.print("              ");
}

void EkranaYazAltMenu3a() {

  if (MenuDegisimDurumu() == true) {

    lcd.clear();
    ImlecIlkYer(secenekler[1]);
    lcd.setCursor(3, 0);
    lcd.print(F("Sesi kapat"));
    lcd.setCursor(3, 1);
    lcd.print(F("Geri"));
    lcd.setCursor(0, 3);
    lcd.print(F("SU ANDA SES ACIK"));
  }
}

void EkranaYazAltMenu3b() {

  if (MenuDegisimDurumu() == true) {

    lcd.clear();
    ImlecIlkYer(secenekler[1]);
    lcd.setCursor(3, 0);
    lcd.print(F("Sesi ac"));
    lcd.setCursor(3, 1);
    lcd.print(F("Geri"));
    lcd.setCursor(0, 3);
    lcd.print(F("SU ANDA SES KAPALI"));

  }
}

void sesAcildiEkrani() {
  digitalWrite(BUZZER_PIN, HIGH);
  lcd.clear();
  lcd.print(F("SES ACILDI"));
  lcd.setCursor(3, 1);
  delay(300);
  lcd.noBacklight();
  delay(150);
  lcd.backlight();
  delay(300);
  lcd.noBacklight();
  delay(150);
  lcd.backlight();
  digitalWrite(BUZZER_PIN, LOW);
}

void sesKapandiEkrani() {
  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print(F("SES KAPATILDI"));
  delay(300);
  lcd.noBacklight();
  delay(150);
  lcd.backlight();
  delay(300);
  lcd.noBacklight();
  delay(150);
  lcd.backlight();
}


void cildir() {
  UidByteSifirla();
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("LUTFEN MikroSD TAKIN"));
  lcd.setCursor(0, 2);
  lcd.print(F("VE AYNI KARTI"));
  lcd.setCursor(0, 3);
  lcd.print(F("BIR KERE DAHA OKUTUN"));
  digitalWrite(BUZZER_PIN, HIGH);
  SD.begin(SD_CS_PIN);
  if (SD.begin(SD_CS_PIN)) {
    myFile = SD.open(txtIsmi, FILE_WRITE);
    if (myFile) {
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}
