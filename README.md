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
        
        print("Hiz: ");
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
    num z = 19.99; // ondalıklı sayı
    char a = 'A'; // bir karakter

    var o = 78;
    var e = "Rafadan Tayfa";
    var t = 0.4;
    // var ifadesi joker gibi bir şey istediğiniz değişken türü için kullanabilirsiniz

### Koşullar ve Döngüler
Şimdilik sadece "if" ve "while" var

    num yakit = 100;
    num tuketimHizi = 15;
    num kritikEsik = 30;
    
    println("--- Ucus Basladi ---");
    
    while (yakit > 0) {
        print("Mevcut Yakit: ");
        println(yakit);
    
        // Kritik yakıt kontrolü
        if (yakit <= kritikEsik) {
            println("UYARI: Yakit seviyesi kritik!");
        }
    
        // Yakıt tüketimi
        yakit = yakit - tuketimHizi;
    
        // Eğer yakıt sıfırın altına düşerse sıfıra sabitle (Negatif yakıt olmaz)
        if (yakit < 0) {
            yakit = 0;
        }
    }
    
    println("MOTOR DURDU: Yakit tukendi.");

### Giriş/Çıkış
Tek bir komut var şimdilik.

    println("Selamünaleyküm Dünya")

Normalde println() komutunun adından da anlaşılacağı gibi yeni bir satıra yazdırması gerekiyordu. Ancak şimdilik böyle kaldı. İlerde print işlevini ekleyince bunu olması gerektiği haline getirmeye çalışırız belki.

### Fonksiyonlar
Daha eklenmedi
