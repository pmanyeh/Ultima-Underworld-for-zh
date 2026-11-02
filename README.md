# 地下創世紀 繁體中文版（Ultima Underworld 1＋2 zh-TW）

《**Ultima Underworld: The Stygian Abyss**（地下創世紀：冥河深淵）》與《**Ultima Underworld II: Labyrinth of Worlds**（地下創世紀 II：萬界迷宮）》的**繁體中文化專案**。
以原版 DOS 遊戲還原的原始碼為基礎，做成能直接在 Windows、macOS 上執行的原生版本，**不需要 DOSBox**。

![啟動頁：選擇要玩的遊戲](docs/release/zh-v1.0/launcher.png)

> **需要你自己擁有的 GOG 版 Ultima Underworld 1＋2。**
> 本專案不含任何原版遊戲檔案，也不含遊戲的英文原文；發佈包只放中文譯文與程式。

## 特色

- **兩款遊戲、同一個入口**：啟動頁選擇 UW1 或 UW2，自動尋找 GOG 安裝位置。
- **全面中文化**：對話、書本、卷軸、物品與戰鬥訊息、介面文字都是繁體中文，採高解析中文字型（粉圓體）。
- **對照攻略不吃力**：人名、地名第一次出現時加註英文原名；需要打字詢問的話題，打中文或英文都可以。
- **中文地圖註記**：在地圖上點一下就能用中文寫註記。
- **原汁原味為預設**：現代按鍵（WASD）、滑鼠視角、血瓶／法力瓶數值、傷害數字、全地圖、無敵等選用功能**全部預設關閉**，想用再到設定畫面（Windows **F11**、Mac **Cmd+,**）打開。
- 支援 Roland MT-32 音樂（需自備 ROM，見包內 `說明_MT-32音樂.txt`）。

| 對話 | 遊戲畫面 |
| --- | --- |
| ![對話](docs/release/zh-v1.0/dialogue.png) | ![遊戲畫面](docs/release/zh-v1.0/game.png) |
| ![地圖](docs/release/zh-v1.0/map.png) | ![設定畫面](docs/release/zh-v1.0/settings.png) |

## 下載與遊玩

1. 到 [Releases](https://github.com/pmanyeh/Ultima-Underworld-for-zh/releases) 下載 `Underworld-zhTW-…-Windows.zip` 或 `…-macOS.zip`。
2. 完整解壓縮。Windows 雙擊 `Underworld.exe`；Mac 雙擊 `開始遊戲.command`。
   - Windows 出現「Windows 已保護您的電腦」：按「其他資訊」→「仍要執行」。
   - Mac 若擋下程式：「系統設定」→「隱私權與安全性」→「仍要打開」。
3. 在啟動頁選擇遊戲。找不到 GOG 安裝位置時，會請你選擇遊戲資料夾。

操作按鍵請看遊戲內設定畫面的「按鍵說明」頁籤，它會列出你目前設定下實際可用的按鍵。

## 專案內容

| 路徑 | 說明 |
| --- | --- |
| [`uw1/`](uw1/README.md)、[`uw2/`](uw2/README.md) | 兩款遊戲的還原原始碼與原生移植 |
| `uw1/i18n/`、`uw2/i18n/` | 中文化資料：詞表、用詞決策、工具與驗證器（譯文以雜湊 catalog 發佈，不含英文原文） |
| [`uw1/docs/i18n/`](uw1/docs/i18n/)、[`uw2/docs/i18n/`](uw2/docs/i18n/) | 各階段的設計、報告與驗證紀錄 |
| [`launcher/`](launcher/) | 雙遊戲啟動頁與 Windows／macOS 打包工具 |
| `exhume/` | 共用執行環境（子模組，[Exhume-for-zh](https://github.com/pmanyeh/Exhume-for-zh)），含 UTF-8、CJK 排版、字型與 HD 文字繪製 |
| [`UPSTREAM_README.md`](UPSTREAM_README.md) | 上游專案原版說明（建置、除錯、移植細節） |

自行建置請參考 [docs/BUILDING.md](docs/BUILDING.md) 與各遊戲目錄內的說明。

## 回報問題與譯文意見

遊戲很長，我們還沒有把每條路線都玩過。遇到問題或譯文不順，請到 [Issues](https://github.com/pmanyeh/Ultima-Underworld-for-zh/issues) 回報，並附上：

- UW1 或 UW2、發生了什麼事、怎麼操作的；
- 截圖（翻譯問題請引用那句話）；
- 遊戲資料夾 `recordings` 內最新一份錄影（可原樣重現你的操作）。

## 致謝

- **[Underworld Exhumed](https://github.com/abedegno/underworld-exhumed)**（abedegno）：原版程式的位元組級還原與原生移植，本專案建立在它之上，並持續與上游同步。
- **UltimaHacks**（John Glassmyer）與 **uwpatch**（SiENcE）：部分強化功能的參考。
- **粉圓體 jf-openhuninn**：遊戲使用的中文字型（SIL OFL 授權，授權文字附於發佈包 `licenses/`）。
- 所有提供譯文、試玩與回報的朋友。

## 授權與聲明

本專案為粉絲研究計畫，與 Electronic Arts、Origin Systems、Blue Sky Productions、Looking Glass Technologies 無任何隸屬或背書關係。
Ultima 與 Ultima Underworld 為 Electronic Arts 的商標，遊戲著作權屬其所有人。
程式碼依 [MIT 授權](LICENSE)，該授權不授予原版遊戲的任何權利。詳見 [NOTICE](NOTICE)、`uw1/NOTICE`、`uw2/NOTICE`。
