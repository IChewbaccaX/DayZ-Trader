# CoreTrader – Trader-Basis für DayZ-Server

Ein schlanker, eigenständiger Trader-Mod (Enforce Script, keine Abhängigkeiten außer Vanilla DayZ).
Er bildet die Grundfunktionen moderner Trader-Systeme ab und ist als Basis zum Weiterbauen gedacht.

## Funktionen

| Funktion | Umsetzung |
|---|---|
| Kaufen / Verkaufen / Alles verkaufen | Trader-Menü mit Kauf-, Verkaufs- und „Alles verkaufen“-Button; der Server prüft alles verbindlich |
| Item-Varianten | `Variants` pro Artikel (z. B. Farben), gleicher Preis, im Menü auswählbar |
| Smart Attachments | `Attachments` pro Artikel; optional direkt mit angebaut kaufen (Preis = Summe) |
| Eigene Währung | Beliebige Items als Geld (`Currencies` in `Settings.json`), Wechselgeld automatisch. Mitgeliefert: `CT_Dollar_1/10/100` |
| Banking | Eigenes Bankkonto pro Spieler (JSON), Ein-/Auszahlen am Trader, Bezahlen/Erlös per Bank |
| Safezones | Kreiszonen; drinnen kein Schaden, Benachrichtigung beim Betreten/Verlassen |
| Verkaufsautomaten | Beliebiges Objekt als Trader (gespawnt oder bereits auf der Map vorhandenes) |
| Echte NPC-Trader | Gespawnte Survivor mit Kleidung, unverwundbar |
| JSON-Konfiguration | Wird beim ersten Start automatisch mit Beispielen erzeugt |

## Installation

1. Den Ordner `CoreTrader` als PBO packen (z. B. mit *Addon Builder*; Prefix `CoreTrader`).
2. In `@CoreTrader/Addons/` legen, Key signieren und auf Server **und** Client laden (`-mod=@CoreTrader`).
3. Server einmal starten → unter `profiles/CoreTrader/` entstehen `Settings.json`, `Traders.json`, `Market.json`.
4. Dateien anpassen, Server neu starten.

## Konfiguration

### `Settings.json`

```json
{
  "Currencies": [ { "ClassName": "CT_Dollar_100", "Value": 100 }, { "ClassName": "CT_Dollar_1", "Value": 1 } ],
  "EnableBanking": 1,
  "BankStartBalance": 0,
  "EnableSellAll": 1,
  "AllowSellRuined": 0,
  "MaxTradeDistance": 5.0,
  "EnableSafeZones": 1,
  "SafeZoneCheckInterval": 2.0,
  "SafeZones": [ { "Name": "Green Mountain", "Position": [3706, 402, 5986], "Radius": 150 } ]
}
```

Statt der mitgelieferten Dollar kann jedes stapelbare Item (auch aus anderen Mods) als Währung dienen.

### `Traders.json`

```json
{
  "Traders": [
    {
      "Name": "Waffenhaendler",
      "Type": "NPC",
      "ClassName": "SurvivorM_Mirek",
      "Position": [3700, 402, 5980],
      "Orientation": [90, 0, 0],
      "SpawnObject": 1,
      "Loadout": ["TacticalShirt_Black", "CargoPants_Black"],
      "Categories": ["Waffen", "Munition"]
    }
  ]
}
```

- `Type`: `NPC` oder `VENDING` (nur Anzeige/Organisation).
- `SpawnObject: 0` + leerer `ClassName` → ein bereits auf der Map stehendes Objekt (z. B. Getränkeautomat) an dieser Position wird zum Trader (Toleranz 2,5 m).
- Die Trader-ID ist die Position in der Liste – Reihenfolge nach Livegang möglichst nicht ändern.

### `Market.json`

```json
{
  "Categories": [
    {
      "Name": "Waffen",
      "Items": [
        { "ClassName": "M4A1", "BuyPrice": 4000, "SellPrice": 1600, "Variants": [], "Attachments": ["M4_OEBttstck", "M4_PlasticHndgrd"] },
        { "ClassName": "Mosin9130", "BuyPrice": 1200, "SellPrice": 500, "Variants": ["Mosin9130_Black", "Mosin9130_Camo"], "Attachments": [] }
      ]
    }
  ]
}
```

- `BuyPrice` / `SellPrice` = `-1` → nicht kaufbar / nicht verkaufbar.
- Attachments brauchen einen eigenen Eintrag mit `BuyPrice` (in irgendeiner Kategorie), sonst werden sie ignoriert.
- Verkaufswert: Munition und Mengen-Items anteilig nach Füllstand; Anbauten und Inhalt werden mitbewertet. Enthält ein Item etwas, das der Trader nicht ankauft, wird der Verkauf abgelehnt (nichts geht verloren).
- „Alles verkaufen“ verkauft nur Inhalt der getragenen Kleidung/Rucksäcke – nicht die Waffe in der Hand oder auf dem Rücken.

## Aufbau

```
CoreTrader/
├─ config.cpp                       Mod-Definition + Dollar-Items
├─ GUI/layouts/trader_menu.layout   Trader-Menü
└─ Scripts/
   ├─ 3_Game/   Datenklassen (JSON), Markt-Lookup, Hilfsfunktionen, RPC-IDs
   ├─ 4_World/  Server-Logik, Geld, Bank, Wertberechnung, Aktion "Handeln", PlayerBase
   └─ 5_Mission/ Server-Init, Client-Init, Menü
```

Ablauf: Client lädt die Konfiguration per RPC → Aktion „Handeln“ am Trader → Server prüft Distanz und öffnet das Menü → jede Kauf-/Verkaufsanfrage wird serverseitig gegen die eigene Konfiguration validiert.

### Andere Banksysteme anbinden

`CT_BankProvider` ableiten und in einer eigenen `modded class MissionServer` nach `super.OnInit()` registrieren:

```c
CT_TraderServer.Get().SetBankProvider(new MyLBBankProvider());
```

## Ideen für den Ausbau

Lagerbestand/Stock, dynamische Preise, Fahrzeughändler mit Spawnpunkten, Suchfeld und Item-Vorschau im Menü, Trader-Rollen (z. B. nur für bestimmte Gruppen), Logging von Transaktionen.

## Hinweis

Der Code ist gegen die aktuellen Vanilla-Skripte (DayZ-Script-Diff) geschrieben, aber noch nicht im Spiel getestet. Beim ersten Start die Skript-Logs (`profiles/script_*.log`) prüfen.
