# Programovací šablona pro studenty

 Vaším úkolem je navrhnout a naimplementovat autonomního agenta (bota), který dokáže co nejrychleji projít bludištěm až do cíle, přičemž se musí vyrovnat s překážkami, omezeným zorným polem a ostatními protihráči.

Projekt je napsaný v C++ a k vykreslování využívá knihovnu **SFML**.

---

## 🎯 Cíl hry a pravidla

1. **Hlavní cíl a bodování:** 
   - Dostat se k **cílovému portálu (Exit)**, který je zobrazen jako rotující zelený portál na mapě.
   - Kolo vyhrává hráč, který dosáhne portálu jako první. Tím kolo ukončí a získá pro svůj tým **10 bodů**.
   - Body se ukládají do tabulky (Leaderboard) na serveru napříč všemi odehranými koly.
2. **Spawnování (Začátek a oživení):**
   - Všichni hráči se na začátku kola objevují v "safe zóně" (prázdná místnost 3x3 dlaždice v levém horním rohu bludiště).
   - Pokud je agent eliminován zbraní, dostává penalizaci **3 sekundy**, během kterých nehraje, a následně se **respawnuje zpět do počáteční safe zóny**.
3. **Interakce a boj:** 
   - Hráči začínají s **0 náboji** a **100 HP**.
   - Zásah střelou ubírá **50 HP** (agent přežije 1 zásah, druhý jej eliminuje).
   - **Bodování boje:** Za eliminaci soupeře získává střelec **+2 body**. Eliminovaná oběť naopak **-2 body** ztratí.
   - Na mapě je k dispozici munice (žluté krystaly). Sebrání přidá **5 nábojů**.
   - Na mapě se v každý moment nachází **přesně tolik kusů munice, kolik je připojeno hráčů**. Po sebrání se munice ihned znovu objeví na jiném náhodném místě.

---

## ⚙️ Jak hra funguje

- **Autoritativní UDP Server:** Server řídí celý stav hry, generuje bludiště a počítá fyziku. Klienti posílají své akce a přijímají stav hry.
- **Omezené zorné pole (Fog of War):** Server posílá vašemu agentovi pouze informace o objektech, které jsou v jeho přímé viditelnosti (Line-of-Sight). Vše, co je za překážkami, je skryto.
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
* `EntityType::Wall` — Blok zdi (velikost dlaždice je 80x80 pixelů).
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
