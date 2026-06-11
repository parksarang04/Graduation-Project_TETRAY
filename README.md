# 🕹️ TETRAY (Tetris with Raylib)

`raylib`과 `C++20` 표준을 활용하여 구현한 크로스 플랫폼 뗴트리스 게임 프로젝트입니다.  
원작 리포지토리를 기반으로 커스텀 디지털 신호처리(DSP) 오디오 합성 엔진 및 안정적인 오디오 자원 관리 아키텍처를 추가하여 포크(Fork) 및 고도화하였습니다.

---

## 🚀 실행 및 다운로드

코드를 직접 빌드하지 않고 게임을 바로 실행해 보시려면 아래 링크에서 실행 파일을 다운로드할 수 있습니다.

* **[📥 최신 실행 파일(.exe) 다운로드]**(https://github.com/parksarang04/TETRAY-main/releases)
  * *주주의: Windows 환경에서 실행 가능하며, 사운드 드라이버 안전장치가 적용되어 있습니다.*

---

## 🛠️ 개발 환경 및 사용 기술

* **Language:** C++20
* **Graphics & Audio Library:** [raylib](https://github.com/raysan5/raylib) (FetchContent를 통한 자동 의존성 주입)
* **CLI Parser:** argparse
* **Build System:** CMake (Version 3.14 이상 권장)
* **IDE:** Visual Studio 2022 (MSVC Compiler)

---

## ✨ 주요 구현 및 개선 사항

1. **테트리스 핵심 메커니즘:**
   * SRS (Super Rotation System) 회전 시스템 알고리즘 적용
   * Hold(홀드), Preview(고스트 미노), 하드 드롭 및 소프트 드롭 완벽 구현
   * 7-Bag 랜덤 제너레이터 큐(Queue) 시스템 탑재

2. **사운드 합성 및 오디오 안전성 강화 (개선):**
   * `miniaudio` 기반의 실시간 디지털 신호처리(DSP) 8비트 사이버펑크 사이드 베이스라인 오디오 합성 콜백 구현
   * 오디오 디바이스의 중복 초기화 방지 및 예외 처리를 통한 런타임 읽기 액세스 위반(Nullptr) 크래시 해결
   * 게임 종료 시 안전한 메모리/스레드 자원 해제(Unload 및 Close 소멸자 아키텍처) 적용

---

## 🕹️ 게임 조작법

* **좌우 이동:** `←` / `→` 방향키
* **소프트 드롭:** `↓` 방향키
* **하드 드롭:** `Space` 바
* **블록 회전:** `Z` (반시계 방향) / `X` (시계 방향)
* **홀드 (Hold):** `C` 또는 `Shift` 키

---

## 💻 빌드 및 실행 방법 (개발자용)

본 리포지토리는 CMake를 사용하므로 외부 라이브러리를 수동으로 설치할 필요가 없습니다.

1. 본 리포지토리를 클론하거나 압축을 해제합니다.
   ```bash
   git clone [https://github.com/parksarang04/TETRAY-main.git](https://github.com/parksarang04/TETRAY-main.git)
