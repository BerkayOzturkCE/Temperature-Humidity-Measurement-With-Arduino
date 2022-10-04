#include <EEPROM.h>
#include <dht11.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 7, 8, 9, 10); //LCD pinleri ayarlandı

//Değişkenler ve Pinler tanımlandı
int Kled = 5, Yled = 6, buzzer = 4, dht11pin = 13;
int PotSayfa = A5;
int sayfa = 0, yenizmn = 0, eskizmn = 0;
int ScklkSnr = 100;
int nemSnr = 100;
String Sbrm = " C";
bool altMenu = false;
dht11 dht;
float nem, sicaklik;



void setup() {
  //led ve buzzer pinleri çıkış pini olarak ayarlandı
  pinMode(Kled, OUTPUT);
  pinMode(Yled, OUTPUT);
  pinMode(buzzer, OUTPUT);

  //EEPROM'dan veriler değişkenlere atandı
  ScklkSnr = EEPROM.read(10);
  nemSnr = EEPROM.read(20);
  int brm = EEPROM.read(30);
  if (brm == 0) {
    Sbrm = " C";
  } else {
    Sbrm = " F";

  }
  Serial.println(Sbrm);

  lcd.begin(16, 2);// LCD başlatıldı
  Menu();
  Serial.begin(9600);//Seri haberleşme başlatıldı

  //Harici kesmeler tanımlandı
  attachInterrupt(1, ButtonClick, RISING);
  attachInterrupt(0, Geri, RISING);

}

void loop() {
  int Sdeger;
  dht.read(dht11pin);
  yenizmn = millis(); //millis fonksiyonundan veri alındı.
  if (yenizmn - eskizmn > 2000) { //Eğer 2 saniye geçtiyse.
    eskizmn = yenizmn;
    //Sensörden veriler alındı ve değişkenlere atandı.
    if (Sbrm == " F") {
      sicaklik = (float)dht.fahrenheit();

    } else {
      sicaklik = (float)dht.temperature;

    }
    nem = (float)dht.humidity;
  }
  if (sicaklik > ScklkSnr || nem > nemSnr) {//Sıcaklık veya nem değeri sınır değeri geçtiyse.
    //Buzzer ve kırmızı led yandı. Yeşil led kapandı.
    digitalWrite(Kled, HIGH);
    digitalWrite(Yled, LOW);
    digitalWrite(buzzer, HIGH);

  } else {
    //Yeşil led yandı. Kırmızı led ve buzzer kapandı.
    digitalWrite(Kled, LOW);
    digitalWrite(Yled, HIGH);
    digitalWrite(buzzer, LOW);
  }

  if (altMenu == false) {//Alt menu false ise.
    Sdeger = analogRead(PotSayfa);//Potansiyometreden değer alındı.
    Serial.println(Sdeger);
    //Sdegere göre hangi sayfada olduğumuz belirlendi
    if (Sdeger < 255) {
      sayfa = 0;
      Menu();
    } else if (Sdeger < 510) {
      sayfa = 1;
      Menu();
    } else if (Sdeger < 765) {
      sayfa = 2;
      Menu();
    } else if (Sdeger < 1023) {
      sayfa = 3;
      Menu();
    }
  } else {
    //Hangi guncelleme sayfasında oldugumuz belirleniyor.
    if (sayfa == 4) {
      SicaklikAyar();
    } else if (sayfa == 5) {
      NemAyar();
    } else if (sayfa == 6) {
      SicaklikBirim();
    }
  }
  delay(200);
}

void Menu() {
  lcd.clear();
  //Sayfa numarasına göre lcd'de ne yazılacağı belirleniyor.
  if (sayfa == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Sicaklik:");
    lcd.print(sicaklik);
    lcd.setCursor(14, 0);

    lcd.print(Sbrm);
    lcd.setCursor(0, 1);
    lcd.print("Nem:");
    lcd.print("%");
    lcd.print(nem);

  } else if (sayfa == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Sicaklik Siniri");
    lcd.setCursor(0, 1);
    lcd.print("Degistir");
  }
  else if (sayfa == 2) {
    lcd.setCursor(0, 0);
    lcd.print("Nem Siniri");
    lcd.setCursor(0, 1);
    lcd.print("Degistir");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Sicaklik Birim");
    lcd.setCursor(0, 1);
    lcd.print("Degistir");
  }
  delay(100);
}

void ButtonClick() {
  if (altMenu == false) {//Eğer altmenu false ise
    altMenu = true; //Altmenu true yapıldı.
    //düğme basıldığında hangi sayfadaysa ona göre yeni sayfa belirlendi.
    if (sayfa == 1) {
      sayfa = 4;
    }
    if (sayfa == 2) {
      sayfa = 5;
    } else if (sayfa == 3) {
      sayfa = 6;
    }
  }
  else  //eğer altmenu true ise
  {
    //Sayfa numarasına göre yeni değerler EEPROM'a kaydediliyor.
    double PotDeger = analogRead(PotSayfa);
    if (sayfa == 4) {
      PotDeger = (PotDeger / 1023) * 100; //Potansiyometreden gelen değer yüzdelik birime çevriliyor.
      ScklkSnr = PotDeger; //Sıcaklık sınır değeri belirlendi.
      EEPROM.write(10, PotDeger); //EEPROM'a kaydedildi

    }
    if (sayfa == 5) {
      PotDeger = (PotDeger / 1023) * 100; //Potansiyometreden gelen değer yüzdelik birime çevriliyor.
      nemSnr = PotDeger;  //nem sınır değeri belirlendi.
      EEPROM.write(20, PotDeger); //EEPROM'a kaydedildi

    } else if (sayfa == 6) {
      if (PotDeger < 512) { //eğer potansiyometreden gelen değer 512den küçükse
        Sbrm = " C"; // Santigrat olarak atandı
        EEPROM.write(30, 0);  //EEPROM'a kaydedildi
      } else {
        Sbrm = " F"; //Fahrenheit olarak atanıyor
        EEPROM.write(30, 1);  //EEPROM'a kaydedildi

      }
    }
    Geri();
  }
}

void Geri() {//geri butonuna basılırsa altmenu false yapılıyor
  altMenu = false;
  sayfa = 0;
}

void SicaklikAyar() {//Sayfa 4
  //Sıcaklık Ayar sayfası
  String metin = "";
  lcd.clear();
  double PotDeger = analogRead(PotSayfa);
  PotDeger = (PotDeger / 1023) * 100;
  metin += PotDeger;
  metin += Sbrm;
  lcd.setCursor(0, 0);
  lcd.print("Yeni Sinir");
  lcd.setCursor(0, 1);
  lcd.print(metin);
  delay(200);
}

void NemAyar() { //Sayfa 5
  //Sıcaklık Ayar sayfası
  String metin = "";
  lcd.clear();
  double PotDeger = analogRead(PotSayfa);
  PotDeger = (PotDeger / 1023) * 100;
  metin += "%";
  metin += PotDeger;
  lcd.setCursor(0, 0);
  lcd.print("Yeni Sinir");
  lcd.setCursor(0, 1);
  lcd.print(metin);
  delay(200);

}

void SicaklikBirim() { // Sayfa 6
  //Sıcaklık Ayar sayfası
  String metin = "";
  lcd.clear();
  double PotDeger = analogRead(PotSayfa);
  lcd.setCursor(0, 0);
  lcd.print("Yeni Birim");
  lcd.setCursor(0, 1);
  if (PotDeger < 512) {
    lcd.print("C");
  } else {
    lcd.print("F");

  }
  delay(200);
}
