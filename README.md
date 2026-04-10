# Üzüm
Üzüm C++ ile geliştirilmiştir. Matematiksel ve Fiziksel hesaplamalar için geliştirilecek. Şuanki sürümü baya erken bir sürüm olduğu için çok fazla şeyi yok.

.uz dosya uzantısında çalıştırabilirsiniz

##Kurulum
Bir VS Code eklentisi mevcuttur. Bu repodaki .vsix dosyasını indirip VS Code'a eklenti olarak kurabilirsiniz. Daha sonra yapmanız gereken tek şey bir .uz dosyası oluşturup kodunuzu yazmak. Eklenti sayesinde sağ üstteki düğmeden kodu hemen çalıştırabilirsiniz.

Örnek kod
´´´

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
´´´
