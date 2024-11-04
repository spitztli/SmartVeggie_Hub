# SmartVeggie_Hub

Projektübersicht

SmartVeggie_Hub ist ein IoT-Projekt, das darauf abzielt, Umweltdaten von zwei verschiedenen Quellen zu erfassen und zentral auf einem LCD-Display darzustellen. Das System verwendet zwei Arduino Uno Mikrocontroller als Sender und einen als Empfänger. Jeder Sender ist mit einem LoRa-Modul ausgestattet, das es ermöglicht, Messdaten über lange Distanzen zu übertragen.

Sender

Wetterstation: Dieser Sender erfasst die Umgebungstemperatur und Luftfeuchtigkeit. Die Daten dienen zur Überwachung der Wetterbedingungen.

Pflanzenüberwachungsstation: Der zweite Sender misst die Bodenfeuchtigkeit, Luftfeuchtigkeit und Temperatur in unmittelbarer Nähe der Pflanzen. Diese Daten sind entscheidend für die Pflege und das Management der Pflanzengesundheit.

Empfänger

Datenzentrale: Der Arduino Uno Empfänger sammelt die Daten von beiden Sendern mittels LoRa-Kommunikation. Die empfangenen Daten werden verarbeitet und kontinuierlich auf einem angeschlossenen LCD-Display angezeigt.

Technische Details

Mikrocontroller: Arduino Uno

Kommunikation: LoRa-Module

Anzeige: LCD-Display

Sensoren: Temperatur, Luftfeuchtigkeit, Bodenfeuchtigkeit
