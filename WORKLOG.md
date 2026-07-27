# WORKLOG

작업 브리핑/면접 준비용 기록. **Claude가 작성한 것**과 **본인이 직접 판단·해결한 것**을 구분해서 남긴다.
사소한 승인("네 진행해주세요")은 기록하지 않고, 실제로 진단/결정/트러블슈팅한 것만 남긴다.

---

## 2026-07-27 — WebSocket 센서 데이터 수직 슬라이스

**목표**: 외부 시뮬레이터가 WebSocket으로 보내는 센서 데이터를 UE 5.7이 수신해 로그로 출력하는 최소 데이터 층 (3D/UI 제외).

### Claude 작업
- UE C++ 스캐폴딩 작성
  - `ISensorDataSource` 인터페이스 (OPC UA 등으로 나중에 교체 가능하도록 추상화)
  - `FWebSocketSensorSource` (WebSocket 구현체, JSON 파싱, 3초 재연결 타이머)
  - `USensorSubsystem` (GameInstanceSubsystem, PIE 시 자동 접속·로그 출력)
  - `FactoryTwin.Build.cs`에 `WebSockets`/`Json`/`JsonUtilities` 의존성 추가
- Python 시뮬레이터 (`Sim/simulator.py`) 작성 — EQ-01 temperature/pressure 랜덤워크 + 임계값 초과 로직
- `Sim/run_simulator.bat` 작성 (반복 실행 편의용)
- 실제 UBT 빌드 실행해서 컴파일 에러 검증 및 수정
- 커밋 전 민감정보(인증정보/개인정보) 스캔

### 본인 작업 (트러블슈팅 포함)
- **Python 미설치 진단**: `python --version` 결과가 버전 번호 없이 `Python`만 출력되는 걸 보고 이상함을 감지 → Windows Store 앱 실행 별칭(스텁)이 실제 설치 없이 명령을 가로채는 증상이라는 걸 파악하고 python.org에서 정식 설치 진행
- **Python 설치 관리자 프롬프트 판단**: 설치 중 뜬 "legacy py 커맨드 충돌" / "PATH에 commands 디렉토리 추가" 두 프롬프트에 대해 각각 y/N 선택 직접 결정
- **작업 디렉토리 오류 직접 해결**: `cd sim` 실행 시 `C:\WINDOWS\system32`에 있어서 경로를 못 찾는 에러 발생 → 프로젝트 루트 기준 올바른 경로(`C:\Work\Github\FactoryTwin\Sim`)로 이동해서 해결
- **PIE 동작 검증**: UE 에디터에서 직접 Play 실행, Output Log와 시뮬레이터 콘솔 값을 나란히 비교해서 실제로 일치하는지 육안 확인 (`EQ-01/temperature = 104.81` 등)
- **Git 인증 이슈 해결**: `git push` 시 "Password authentication is not supported" 에러 발생 → GitHub 인증 방식(HTTPS 토큰/CLI)을 직접 설정해서 해결, 이후 push 재확인 요청
- **보안 검토 요청 판단**: 커밋 전에 먼저 민감정보 검증을 요구 — Claude가 자동으로 하지 않던 단계를 본인이 프로세스에 추가
- **반복 작업 효율화 요구**: PowerShell 매번 여는 게 번거롭다고 판단해 `.bat` 스크립트화 요청

---

## 2026-07-27 — 3D 씬 시각화 슬라이스

**목표**: 설비(EQ-01) 액터를 레벨에 배치하고, 수신되는 센서 값에 따라 색상(상태등)/텍스트로 시각화. 수동 레벨 편집 없이 코드만으로 재현 가능하게.

### Claude 작업
- `USensorSubsystem`에 `OnSensorDataReceived()` 델리게이트 추가 (기존 로그 출력에 더해 3D 액터/UI가 구독할 수 있도록 재브로드캐스트)
- `AEquipmentActor` 작성 (`Visualization/EquipmentActor.h/.cpp`)
  - `UStaticMeshComponent`(엔진 기본 Cube) + `UPointLightComponent`(상태등) + `UTextRenderComponent`(수치 표시), 전부 코드 생성이라 커스텀 머티리얼/에셋 불필요
  - temperature 값에 따라 초록(정상)/호박색(경고, 85 이상)/빨강(위험, 100 이상)으로 상태등 색상 전환
  - `EquipmentId`로 자기 자신에게 해당하는 데이터만 필터링해서 반응
- `UFactoryTwinWorldSubsystem` 작성 (`WorldSubsystem`, `OnWorldBeginPlay`에서 `AEquipmentActor` 자동 스폰) — GameMode/레벨 설정을 안 건드리고도 PIE 시작하면 항상 액터가 뜨도록 설계 (레벨에 수동 배치 불필요)
- 빌드 검증 완료 (UBT 컴파일 성공)
- **`run_simulator.bat` 인코딩 버그 진단/수정**: 더블클릭 시 반응 없음 → PowerShell에서 직접 실행하니 `run_simulator : ...로 인식되지 않습니다` → `.\run_simulator.bat`으로 재실행하니 이번엔 `명령 구문이 올바르지 않습니다` 에러. 원인은 배치 파일 안 한글 안내 메시지가 UTF-8로 저장돼 있어서 cmd.exe가 콘솔 코드페이지(CP949)로 잘못 해석, `if (...)` 블록 구조가 깨진 것. 배치 파일 텍스트를 영어로만 교체해서 인코딩 문제 자체를 제거

### 본인 작업 (트러블슈팅 포함)
- **`run_simulator.bat` 실행 안 되는 문제 재현/보고**: 더블클릭 시 반응 없음을 감지하고, 터미널에서 직접 실행해 정확한 에러 메시지(`run_simulator : ... 인식되지 않습니다` → PowerShell 상대경로 이슈, 이후 `명령 구문이 올바르지 않습니다`)를 그대로 캡처해서 전달 → 정확한 재현 스텝 덕분에 근본 원인(배치 파일 한글 인코딩) 진단 가능했음
- **PIE 최종 검증**: 시뮬레이터 콘솔 값(`EQ-01/temperature = 69.86` 등)과 3D 화면의 상태등 색상·텍스트(`Temp: 69.9`)를 직접 비교해서 실시간 반영되는지, 임계값 미만일 때 초록 상태등이 맞는지 육안 확인

---

## 2026-07-27 — UI 위젯 실시간 표시 슬라이스

**목표**: 화면 한쪽에 UMG HUD로 설비 센서 값(온도/압력)을 실시간 텍스트로 표시. 3D 공간 텍스트와 별개로 화면 고정 UI 추가.

### Claude 작업
- `FactoryTwin.Build.cs`에 `UMG`(Public), `Slate`/`SlateCore`(Private) 의존성 추가
- `USensorHudWidget` 작성 (`UI/SensorHudWidget.h/.cpp`)
  - Widget Blueprint 에셋을 에디터에서 만드는 대신, `NativeConstruct()`에서 `WidgetTree->ConstructWidget<>()`로 CanvasPanel + VerticalBox + TextBlock 3줄(제목/Temp/Press)을 전부 코드로 생성 — 에디터 UMG 디자이너 작업 없이 완전히 코드/버전관리로 재현 가능
  - `USensorSubsystem::OnSensorDataReceived()` 구독, `AEquipmentActor`와 동일한 임계값 로직(85=경고/호박색, 100=위험/빨강)으로 텍스트 색상 전환
- `UFactoryTwinWorldSubsystem::OnWorldBeginPlay`에 `CreateWidget<USensorHudWidget>` + `AddToViewport()` 추가 (데디케이티드 서버 넷모드는 스킵) — PIE 시작하면 액터 스폰과 함께 HUD도 자동으로 뜨도록 통합
- 빌드 검증 완료 (UBT 컴파일 성공)
- `Sim/simulator.py`에 `FACTORYTWIN_SIM_FAST` 환경변수 기반 fast mode 추가 (업데이트 주기 0.5~1.0초→0.1~0.2초, 임계값 초과 확률 5%→35%) — 검증 속도를 위해 기본 동작은 안 건드리고 옵션으로 분리
- **`AEquipmentActor` 텍스트 색상 버그 수정 (1차)**: `RefreshVisuals()`가 계산한 `StatusColor`를 상태등(`StatusLightComponent`)에만 적용하고 3D 텍스트(`ReadoutTextComponent`)에는 한 번도 적용 안 하고 있던 걸 발견 → `SetTextRenderColor(StatusColor.ToFColor(true))` 호출 추가해서 텍스트도 온도에 따라 색이 바뀌도록 수정
- **`AEquipmentActor` 텍스트 컴포넌트 분리 (2차)**: 1차 수정 후에도 Press 줄이 Temp와 같은 색으로 바뀌는 문제 확인. 원인은 제목+Temp+Press가 `UTextRenderComponent` 하나에 합쳐진 텍스트라 색상이 통째로 적용되는 구조였던 것 (HUD는 줄마다 별도 위젯이라 원래 문제없었음). `StatusTextComponent`(제목+Temp, 색 변함)와 `PressureTextComponent`(Press, 항상 흰색)로 컴포넌트 자체를 분리해서 해결

### 본인 작업 (트러블슈팅 포함)
- **fast mode 요청**: 값 변화를 매번 오래 기다려서 검증하는 게 비효율적이라 판단, 빈도수를 올려서 빨리 검증하자고 먼저 제안
- **텍스트 색상 버그 발견**: fast mode로 값이 100을 빠르게 넘나드는 걸 보다가, 텍스트 색이 전혀 안 바뀌는 걸 알아채고 버그로 보고 → 실제로 `ReadoutTextComponent` 색상 갱신 누락 확인, 수정으로 이어짐
- **설계 의도 재확인 질문**: temp/press가 같은 색으로 처리되는 게 어색해서 의도인지 재차 질문 → "press는 색상 미반영이 의도"라는 답을 듣고, 그럼 지금 Press도 같이 색이 바뀌는 건 의도와 모순 아니냐고 정확히 짚어냄 → 2차 버그(텍스트 컴포넌트 미분리) 발견으로 이어짐

---
