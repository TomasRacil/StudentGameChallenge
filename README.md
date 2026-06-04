# Game Challenge — Programovací šablona pro studenty

Vítejte v programovací výzvě **Game Challenge**! Vaším úkolem je navrhnout a naimplementovat autonomního agenta (bota), který dokáže co nejrychleji projít bludištěm až do cíle, přičemž se musí vyrovnat s překážkami, omezeným zorným polem a ostatními protihráči.

Projekt je napsaný v C++ a k vykreslování využívá knihovnu **SFML**.

---

## 🎯 Cíl hry

1. **Hlavní cíl:** Dostat se k **cílovému portálu (Exit)**, který je zobrazen jako rotující zelený portál na mapě.
2. **Kritérium vítězství:** Vítězem se stává tým, jehož agent dosáhne portálu v **nejkratším čase** (nejlepší čas se zaznamenává do tabulky na serveru napříč koly).
3. **Interakce a boj:** 
   - Hráči začínají s **0 náboji** a **100 HP** (životy).
   - Zásah nepřátelskou střelou ubere **50 HP**. Agent tedy **přežije pouze 1 zásah**, 2. zásah jej eliminuje a respawnuje zpět na startovní pozici (což přináší výraznou časovou penalizaci).
   - Munici (krabice s náboji) lze sbírat na mapě. Počet krabic s municí na mapě odpovídá počtu připojených hráčů.

---

## ⚙️ Jak hra funguje

- **Autoritativní UDP Server:** Server řídí celý stav hry, generuje bludiště a počítá fyziku. Klienti posílají své akce a přijímají stav hry.
- **Omezené zorné pole (Fog of War):** Server posílá vašemu agentovi pouze informace o objektech, které jsou v jeho přímé viditelnosti (Line-of-Sight) do vzdálenosti maximálně **8 polí**. Vše ostatní je skryto ve tmě.
- **Lobby a kola:** Hra začíná v režimu Lobby. Jakmile učitel na serveru stiskne klávesu **ENTER**, vygeneruje se bludiště a odstartuje se kolo. Po dosažení cíle jedním z hráčů kolo končí, zobrazí se výsledky a po 5 sekundách se hra vrátí zpět do Lobby.

---

## 🧠 Jak naprogramovat agenta

Veškerý váš kód pište do souboru:
📍 **`client/src/MyAgent.cpp`**

Zde upravujete třídu `MyAgent`, která dědí od základní třídy `Agent` a obsahuje tři klíčové metody:

### 1. `std::string getTeamName() const`
Vrátí název vašeho týmu, který se zobrazí na serveru a v žebříčku nejlepších časů.

### 2. `void update(const VisibleState& state)`
Volá se každé síťové okno (60 FPS). Zde obdržíte aktuální stav viditelný pro vašeho agenta:
* `state.myPosition` — Vaše aktuální pozice `(x, y)` na mapě.
* `state.myRotation` — Směr, kterým se díváte (ve stupních: `0` = vpravo, `90` = dolů, `180` = vlevo, `270` = nahoru).
* `state.myHealth` — Vaše aktuální zdraví (0 až 100).
* `state.myAmmo` — Aktuální počet nábojů.
* `state.serverState` — Stav hry (`ServerState::Lobby`, `ServerState::Playing`, `ServerState::RoundEnd`).
* `state.currentRoundTime` — Čas uplynulý v aktuálním kole.
* `state.entities` — Seznam všech objektů, které váš agent aktuálně vidí (zdi, nepřátelé, náboje, cílový portál).

### 3. `Action getAction()`
Vrací akci, kterou má váš bot v tomto kroku provést. Můžete vrátit jednu z následujících akcí:
* `Action::None` — Žádná akce.
* `Action::MoveForward` — Pohyb dopředu ve směru rotace.
* `Action::MoveBackward` — Pohyb dozadu.
* `Action::RotateLeft` — Otočení vlevo (proti směru hodinových ručiček).
* `Action::RotateRight` — Otočení vpravo (po směru hodinových ručiček).
* `Action::PickupAmmo` — Pokus o sebrání krabice s municí (pokud stojíte těsně u ní). Sebrání přidá **5 nábojů**.
* `Action::Shoot` — Výstřel ze zbraně (pokud máte alespoň 1 náboj).

*Poznámka: Výchozí šablona obsahuje manuální klávesové ovládání (W, A, S, D, E pro sběr, Mezerník pro střelbu), abyste si mohli hru vyzkoušet sami.*

---

## 📦 Datové struktury

### Entita (`Entity`)
Každý viditelný objekt v `state.entities` obsahuje:
* `e.type` — Typ entity (viz níže).
* `e.id` — Unikátní identifikátor entity (např. ID nepřítele).
* `e.position` — Pozice entity `sf::Vector2f(x, y)`.
* `e.rotation` — Úhel natočení entity ve stupních.

### Typy entit (`EntityType`)
* `EntityType::Wall` — Blok zdi (velikost dlaždice je 40x40 pixelů).
* `EntityType::Player` — Ostatní nepřátelští hráči.
* `EntityType::Ammo` — Krabice s municí.
* `EntityType::Bullet` — Letící projektil.
* `EntityType::Exit` — Cílový portál (zelený).

---

## 🛠️ Jak projekt zkompilovat a spustit

### 1. Prvotní sestavení (pomocí CMake)
Otevřete terminál (PowerShell) ve složce projektu a spusťte:
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### 2. Kopírování DLL souborů SFML (pouze poprvé)
SFML vyžaduje sdílené knihovny `.dll` ve stejné složce jako jsou spustitelné soubory:
```powershell
Copy-Item "build\_deps\sfml-build\lib\Release\*.dll" -Destination "build\server\Release\"
Copy-Item "build\_deps\sfml-build\lib\Release\*.dll" -Destination "build\client\Release\"
```

### 3. Spuštění hry
1. **Server (spouští učitel):**
   ```powershell
   .\build\server\Release\server.exe
   ```
2. **Klient (spouští studenti):**
   ```powershell
   .\build\client\Release\client.exe
   ```
   *Po spuštění klienta zadejte do konzole název svého týmu.*

Jakmile se všichni připojí, učitel stiskne **ENTER** na okně serveru a hra začíná!
