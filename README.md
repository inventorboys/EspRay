# EspRay
EspRay Projesine Hoş Geldiniz
malzemeler 2 adet esp32 (32u olursa daha iyi olur dahili anten ile) grafik izlemek ve kod yüklemek için bilgisayar 
ilk önce alıcıya kodu yükleyin seri portu açın bu sayı kibi bir şey görmelisiniz 1C:C3:AB:B8:04:28
o sayıyı vericinin kodundaki bu kısma yazın uint8_t ALICI_MAC[] = {0x1B, 0xC3, 0xAB, 0xB8, 0x04, 0x28};    0x1B, 0xC3, 0xAB, 0xB8, 0x04, 0x28
sonra alıcı kodunu farklı bir esp 32 ye yükleyin 
Şimdi dosyadaki Hareket İzleme Panelini açın port seçin kısmından esp nin portunu seçin portu seçin bu işlemi sadece alıcı espde yapın 
html dosyası sadece edge ve chrome da çalışır firefox opera vs. de çalışmiyicaktır
bir esp yi duvarın bi tarafın diğer esp yi diğer tarafa koyun arasına bir arkadaşınız geçsin aradan çekilince değer artıcak önüne geçince azalıcak 
ve haraket olarak kırmzı yanıcaktır

