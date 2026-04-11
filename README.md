# Üzüm
Üzüm C++ ile geliştirilmiştir. Matematiksel ve Fiziksel hesaplamalar için geliştirilecek. Şuanki sürümü baya erken bir sürüm olduğu için çok fazla şeyi yok.

.uz dosya uzantısında çalıştırabilirsiniz

## Kurulum
Bir VS Code eklentisi mevcuttur. Bu repodaki .vsix dosyasını indirip VS Code'a eklenti olarak kurabilirsiniz. Daha sonra yapmanız gereken tek şey bir .uz dosyası oluşturup kodunuzu yazmak. Eklenti sayesinde sağ üstteki düğmeden kodu hemen çalıştırabilirsiniz.

Örnek kod:

    num HEDEF_HIZ = 28000;
    num suankiHiz = 0;
    num yakit = 95;
    
    while (suankiHiz < HEDEF_HIZ) {
        suankiHiz = suankiHiz + 5000;
        yakit = yakit - 12;
        
        println("Hiz: ");
        println(suankiHiz);
    
        if (yakit < 20) {
            println("UYARI: Kritik yakit seviyesi!");
        }
    }
    
    if (suankiHiz >= HEDEF_HIZ) {
        println("BASARI: Yorungeye oturuldu.");
    }

## Dil
Bu kısımda Üzüm dilinin nasıl kullanılabileceğini öğrenebilirsiniz.

### Temeller
-If ve while gibi döngüler için süslü yay ayraçlar gerekir ({})

-Sonda noktalı virgül gereklidir (;)

-Yorum satırları iki eğik çizgi ile oluşturulur (//)

### Değişken oluşturma

    int x = 54; // tam sayı
    str y = "İstanbul"; // metin
    bool uzum_en_iyi_dil_mi = true; // mantıksal değişken
    num z = 19.99; // ondalıklı da olabilen her türlü sayı
    char a = 'A'; // bir karakter

    var i = 78;
    var e = "Rafadan Tayfa";
    var t = 0.4;
    // var ifadesi joker gibi bir şey istediğiniz değişken türü için kullanabilirsiniz

### Koşullar ve Döngüler
Şimdilik sadece "if" ve "while" var
else if ve else de var

    num irtifa = 0;
    num hedefIrtifa = 1000;
    num artisMiktari = 200;
    
    println("--- Gökbörü-1 Uçuş Kontrol Sistemi Başlatılıyor ---");
    
    while (irtifa <= hedefIrtifa) {
        print("Mevcut İrtifa: ");
        println(irtifa);
    
        if (irtifa == 0) {
            println("Durum: Pistte, kalkış hazırlığı yapılıyor.");
        } else if (irtifa > 0 and irtifa < 500) {
            println("Durum: Tırmanış devam ediyor, düşük irtifa.");
        } else if (irtifa >= 500 and irtifa < 1000) {
            println("Durum: Seyir irtifasına yaklaşıldı, motorlar stabilize.");
        } else {
            println("Durum: Hedef irtifaya ulaşıldı, otomatik pilot devrede.");
        }
    
        irtifa = irtifa + artisMiktari;
    }
    
    println("--- Uçuş Görevi Tamamlandı ---");

### Giriş/Çıkış
Tek bir komut var şimdilik.

    println("Selamünaleyküm Dünya")

Normalde println() komutunun adından da anlaşılacağı gibi yeni bir satıra yazdırması gerekiyordu. Ancak şimdilik böyle kaldı. İlerde print işlevini ekleyince bunu olması gerektiği haline getirmeye çalışırız belki.

### Fonksiyonlar
aşağıdaki gibi fonksiyon oluşturabilirsiniz

    task topla(x, y) {
        return x + y;
    }

    println(topla(4, 8)); 

### Matematiksel ve Mantıksal işlemler

    // 1. Matematiksel Fonksiyon: Hız ve İrtifa Bazlı Basınç Katsayısı
    task katsayiHesapla(hiz, irtifa) {
        // Karmaşık işlem önceliği testi: (A + B) * C / D - (E % F)
        // Negatif sayı ve ondalıklı sayı desteği de burada test edilir
        num sonuc = (hiz + 150.5) * 0.85 / (irtifa + 1) - (hiz % 7);
        return sonuc;
    }
    
    // 2. Mantıksal Fonksiyon: Uçuş Zarfları (Flight Envelope) Kontrolü
    task guvenliMi(hiz, irtifa, katsayi) {
        // AND, OR, XOR ve Karşılaştırma operatörlerinin devasa zinciri
        bool hizKritik = (hiz > 800 or hiz < 100);
        bool irtifaKritik = (irtifa >= 12000);
        
        // Katsayı 1.0 ile 5.0 arasında mı? (Mantıksal Sıkıştırma)
        bool katsayiNormal = (katsayi >= 1.0 and katsayi <= 5.0);
    
        // XOR Testi: Ya hız kritik olacak ya irtifa, ikisi birden kritikse sistem alarm verir!
        if ((hizKritik xor irtifaKritik) and katsayiNormal == true) {
            return true;
        }
        
        // Eğer katsayı anormal ise veya her iki durum da kritikse false döner
        return false;
    }
    
    // --- ANA TEST PROGRAMI ---
    
    num m_hiz = 450;
    num m_irtifa = 5000;
    num m_sayac = 1;
    
    println("--- Gökbörü-1 Aviyonik Matematik Testi Başlıyor ---");
    
    while (m_sayac <= 3) {
        num guncelKatsayi = katsayiHesapla(m_hiz, m_irtifa);
        bool durum = guvenliMi(m_hiz, m_irtifa, guncelKatsayi);
    
        print("Deney #"); print(m_sayac);
        print(" | Hiz: "); print(m_hiz);
        print(" | Katsayi: "); println(guncelKatsayi);
    
        if (durum == true) {
            println("Sinyal Durumu: Stabil ve Güvenli.");
        } else if (durum == false and m_hiz > 400) {
            println("Sinyal Durumu: Stabil Değil - Katsayı Limit Dışı!");
        } else {
            println("Sinyal Durumu: KRİTİK HATA!");
        }
    
        // Parametreleri her adımda değiştirerek sınırları zorla
        m_hiz = m_hiz + 200;
        m_irtifa = m_irtifa + 4000;
        m_sayac = m_sayac + 1;
    }
    
    println("--- Analiz Tamamlandı ---");
