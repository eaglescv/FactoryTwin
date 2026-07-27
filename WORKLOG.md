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

## 2026-07-27 — 임계값 알람/이벤트 처리 슬라이스

**목표**: 값이 threshold를 넘으면 액터/HUD 색상 변경과는 별개로, 전용 델리게이트/이벤트로 분기해서 경고 로그와 향후 알림 트리거(Slack/Discord 등)의 기반을 마련.

### Claude 작업
- `USensorSubsystem`에 `ESensorAlarmSeverity`(Normal/Warning/Critical) enum + `FOnSensorAlarmChanged` 델리게이트 추가
- 센서별 임계값 테이블(`Thresholds`: temperature 85/100, pressure 45/55)과 `(EquipmentId, SensorKey)` 별 마지막 심각도 상태(`LastSeverityByEquipmentSensor`) 추적
- `HandleSensorData`에서 심각도가 실제로 바뀔 때만(예: Normal→Warning) `[Alarm]` 로그 출력 + `OnSensorAlarmChanged` 브로드캐스트 — 매 틱마다 스팸 로그가 나지 않도록 상태 전이 시점에만 발화
- 빌드 검증 완료 (UBT 컴파일 성공)
- **알려진 중복 이슈 남김**: 이 임계값 테이블(85/100, 45/55)이 `AEquipmentActor`/`USensorHudWidget`가 자체적으로 갖고 있는 `WarningTemperature`/`CriticalTemperature`(85/100)와 별개로 존재 — 지금은 일부러 시각화 코드를 안 건드리고 알람 이벤트 계층만 새로 추가했음. 다음 "구조 개선" 작업 때 하나로 합치는 게 좋음 (TODO 주석으로 남겨둠)

### 본인 작업 (트러블슈팅 포함)
- **로그 색상 = verbosity라는 것 직접 확인**: Output Log에서 `[Alarm]`이 노란색으로 뜨는 걸 보고 "이게 임계값 넘나드는 거냐"고 확인 질문 → UE_LOG를 항상 `Warning` verbosity로 찍고 있어서 Critical이 와도 색은 항상 노랑이라는 것, 즉 로그 색이 비즈니스 심각도와 무관하다는 걸 스스로 짚어냄
- **범위 확정 질문**: "임계치 넘으면 근본적으로 뭘 원했던거냐"고 되물어서, 지금 만든 게 배너/알람 히스토리/외부 알림 같은 실제 예외처리가 아니라 이벤트 배관(delegate+로그)뿐이라는 걸 명확히 하고, 지금은 그 이상 필요 없다고 스코프를 직접 확정함

---

## 2026-07-27 — 구조 개선 슬라이스

**목표**: 지난 알람 작업에서 남겨둔 TODO(임계값 중복) 정리 + WebSocket URL 하드코딩 제거 + 재연결 고정 딜레이 개선 + 다중 설비 확장 준비.

### Claude 작업
- **임계값 단일화**: `AEquipmentActor`/`USensorHudWidget`가 각자 갖고 있던 `WarningTemperature`/`CriticalTemperature` UPROPERTY 삭제. 대신 `USensorSubsystem::GetSeverity()`를 조회해서 심각도 판단, `GetSensorAlarmSeverityColor()` 공용 함수로 색상 변환 — 임계값과 색상 매핑이 이제 한 곳(`SensorSubsystem`)에만 존재
- **설정 분리**: `USensorSubsystem`을 `UCLASS(Config = Game)`로 변경, `ServerUrl`/`KnownEquipmentIds`를 `UPROPERTY(Config)`로 노출. `Config/DefaultGame.ini`에 `[/Script/FactoryTwin.SensorSubsystem]` 섹션 추가 — WebSocket 주소가 더 이상 `.cpp`에 하드코딩되지 않음
- **재연결 지수 백오프**: `FWebSocketSensorSource`의 고정 3초 재연결을 1초 시작 → 실패마다 2배 → 최대 30초 cap으로 개선, 연결 성공 시 딜레이 리셋
- **다중 설비 확장 준비**: `UFactoryTwinWorldSubsystem::OnWorldBeginPlay`가 `KnownEquipmentIds` 배열을 순회하며 설비마다 액터(X축 300 간격 배치)와 HUD(세로 90px 간격 스택)를 자동 생성하도록 변경. `DefaultGame.ini`에 `+KnownEquipmentIds=EQ-02` 한 줄만 추가하면 코드 수정 없이 두 번째 설비 등장
- 빌드 검증 완료 (UBT 컴파일 성공)

- **ini `//` 주석 버그 수정**: PIE 켜니 `[SensorSource] Connection error on ws:: Bad protocol ''`로 재연결 루프에 빠짐 (본인이 PIE로 직접 확인해서 발견). 원인 추적 결과 `Config/DefaultGame.ini`의 `ServerUrl=ws://127.0.0.1:8765`에서 따옴표 없는 `//`를 Unreal ini 파서가 주석 시작으로 해석해 `//` 뒤가 전부 잘려나가 `ws:`만 남은 것 확인 (`ConfigCacheIni.cpp`의 `ShouldExportQuotedString` 주석에 명시된 동작). `ServerUrl="ws://127.0.0.1:8765"`로 따옴표 감싸서 수정
- **UMG HUD 미표시 버그 수정**: ini 버그 고친 후에도 화면 좌상단 UMG HUD가 안 뜨는 것 확인. 원인은 `UFactoryTwinWorldSubsystem::OnWorldBeginPlay` 시점이 게임 뷰포트 서브시스템이 준비되기 전이라 `AddToViewport()`가 조용히 no-op하고 있던 것 (3D 액터는 뷰포트가 필요 없어서 멀쩡했음). HUD 생성 부분만 `FTSTicker` one-shot 타이머로 한 틱 뒤로 미뤄서 해결 (WebSocket 재연결에 썼던 것과 같은 패턴)

### 본인 작업 (트러블슈팅 포함)
- **PIE 재검증으로 설정 분리 회귀 발견**: 구조 개선 후 PIE에서 재연결 에러 로그가 반복되는 걸 직접 확인하고 스크린샷으로 공유 → ini `//` 주석 버그 발견/수정으로 이어짐
- **3D 텍스트와 2D HUD 혼동 질문 → HUD 미표시 버그 발견**: 줌인된 3D 텍스트를 보고 "뭐가 바뀐거야"라고 질문 → 3D 텍스트는 의도대로 동작 중임을 확인하는 과정에서, 원래 있어야 할 좌상단 UMG HUD가 화면 전체 스크린샷에도 전혀 안 보인다는 걸 짚어냄 → 뷰포트 타이밍 버그 발견/수정으로 이어짐

---

## 2026-07-27~28 — UMG HUD 미표시 버그: 근본 원인 추적 (장기 디버깅 세션)

**배경**: "뷰포트 타이밍" 수정(`FTSTicker`로 한 틱 지연) 이후에도 HUD가 여전히 안 보임. `IsInViewport()==true`인데 화면엔 아무것도 안 뜨는 모순적인 상태가 계속됨 — 이번 세션은 이 하나의 버그를 끝까지 추적한 기록. **근본 원인은 Claude의 최초 설계 실수**(아래 결론 참고)였고, 그걸 찾아내는 전체 과정은 거의 전적으로 본인이 직접 주도한 체계적 디버깅이었음.

### 본인 작업 (트러블슈팅 — 이번 라운드의 핵심)
- **가설을 하나씩 직접 세우고 검증**: "World가 의심된다"며 `CreateWidget(World,...)`를 `CreateWidget(GameInstance,...)`로 직접 바꿔서 A/B 테스트 → Watch 창에서 `GameInstance`가 유효한 포인터임을 확인해 이 가설을 스스로 기각
- **VS 디버깅 환경을 직접 구축**: `.uproject` 파일 연결이 깨져서 우클릭 메뉴가 없어진 걸 발견 → `UnrealVersionSelector.exe` 위치를 직접 찾아 실행해서 파일 연결 복구 → "Generate Visual Studio project files" 직접 실행
- **엔진 크래시 대응**: PIE 도중 크래시 발생 → 임시 파일(Binaries/Intermediate/DerivedDataCache) 직접 삭제 → 재구동 중 VS의 "시작 프로젝트가 Class Library"라는 에러를 직접 마주치고 보고
- **브레이크포인트로 직접 스텝인 시도**: `Hud->AddToViewport()`에 브레이크포인트 걸고, "내 코드만" 옵션도 직접 껐음 → 엔진 심볼 부재로 스텝인이 안 되는 것까지 직접 확인
- **가장 결정적인 테스트를 스스로 설계·실행**: 레벨 블루프린트를 직접 열어서(메뉴 위치도 직접 찾음) `Event BeginPlay → Create Widget → Add to Viewport`로 C++ 코드를 완전히 배제한 순수 블루프린트 테스트 구성
  - 1차: 엔진 기본 위젯(`Audio Text Box`)으로 테스트 → **화면에 뜸** (가늘게라도 렌더링 확인)
  - 2차: `Submix Effect Delay Preset Widget`으로도 테스트 → **역시 뜸**
  - 3차: `Class`를 `SensorHudWidget`으로 바꿔서 동일한 블루프린트 경로로 테스트 → **안 뜸**
  - 이 세 가지 비교를 통해 "프로젝트 뷰포트 파이프라인 자체는 정상, `USensorHudWidget` 클래스 자체에 문제가 있다"는 걸 실험적으로 완전히 증명함 — Claude가 엔진 소스를 읽어서 이론적으로 추정만 하던 걸, 실제 A/B 테스트로 확정지은 게 본인임
- **VS에서 C++ 코드도 직접 수정하며 실험**: `FactoryTwinWorldSubsystem.cpp`의 `OnWorldBeginPlay`를 통째로 주석 처리해서 C++ 경로와 블루프린트 경로가 절대 섞이지 않는 깨끗한 테스트 환경을 스스로 만듦
- **수정 적용 후 최종 검증**: 마젠타 디버그 박스 + `EQ-01`/`Temp`/`Press` 텍스트가 화면에 정상적으로 뜨는 것을 PIE로 직접 확인

### Claude 작업
- 최초 `USensorHudWidget` 구현 시 위젯 트리 구성 코드(`BuildLayout()`)를 `NativeConstruct()`에서 호출하도록 설계 — **이게 근본 원인이었음 (Claude의 설계 실수)**
- 디버깅 과정에서 엔진 소스 분석 지원: `UGameViewportSubsystem::AddToScreen()` 호출 체인 추적(`GameViewportSubsystem.cpp`), `AddViewportWidgetContent()`까지 도달하는 것 확인
- 여러 단계의 진단용 로그 추가/제거 (`[FactoryTwinWorldSubsystem]`, `[SensorHudWidget]` 태그)
- 3D 액터/HUD 텍스트를 임시로 주석 처리해서 마젠타 디버그 박스만 남기는 격리 테스트 코드 작성
- **최종 근본 원인을 엔진 소스에서 직접 확인**: `UserWidget.cpp` 1203번째 줄, `UUserWidget::RebuildWidget()`의 기본 구현이 `WidgetTree->RootWidget`이 null이면 빈 `SSpacer`로 대체(`TakeWidget()`의 스냅샷 시점). `RebuildWidget()`은 `NativeConstruct()`보다 먼저 호출되므로, `NativeConstruct()` 안에서 `WidgetTree->RootWidget`을 채우는 건 이미 스냅샷이 끝난 뒤라 아무 효과가 없었던 것
- **수정**: `USensorHudWidget::RebuildWidget()`을 오버라이드해서 그 안에서 `BuildLayout()` 호출 후 `Super::RebuildWidget()` 반환하도록 변경. `NativeConstruct()`는 원래 목적(센서 데이터 구독, 초기 표시 갱신)만 담당하도록 정리
- 디버그 마커(마젠타 박스)와 진단용 로그 제거, `FactoryTwinWorldSubsystem.cpp` 정상 상태로 복원
- 빌드 검증 완료 (UBT 컴파일 성공)

### 결론 / 교훈
C++ 전용(Blueprint 에셋 없이) UMG 위젯을 만들 때는 **위젯 트리 구성을 `NativeConstruct()`가 아니라 `RebuildWidget()` 오버라이드에서 해야 한다**는 게 핵심 교훈. 이건 UE 공식 문서에서도 그렇게 눈에 띄게 강조되지 않는, 꽤 비직관적인 UMG 라이프사이클 함정이라 흔히 겪는 실수임 — 애초에 Claude가 이 프로젝트의 `SensorHudWidget`을 처음 설계할 때(3D 씬 시각화 다음 슬라이스 진행 시) 이 함정에 걸렸던 것.

---

## 2026-07-28 — OPC UA 연동 준비 슬라이스

**목표**: 지금 `ISensorDataSource` 인터페이스가 "나중에 OPC UA로 교체 가능"하다고 처음부터 주장해왔는데, 실제로 다른 구현체를 끼워봐서 그게 진짜인지 검증. 실제 OPC UA 클라이언트 라이브러리(예: open62541) 연동은 서버가 없어서 범위 밖 — 대신 인터페이스 스왑 가능성 자체를 증명하는 목업 구현체를 추가.

### Claude 작업
- `FMockOpcUaSensorSource` 추가 (`DataSource/MockOpcUaSensorSource.h/.cpp`) — `ISensorDataSource` 구현체. 실제 OPC UA 서버에 붙는 대신 내부적으로 온도/압력 값을 랜덤워크하며 자체 생성 (`FTSTicker`로 1초마다). `USensorSubsystem`/`AEquipmentActor`/`USensorHudWidget` 등 소비자 쪽 코드는 전혀 안 건드림 — 그게 이 테스트의 요점
- `USensorSubsystem`에 `ESensorDataSourceType`(WebSocket/OpcUaMock) config 프로퍼티 추가, `Initialize()`에서 이 값에 따라 `FWebSocketSensorSource` 또는 `FMockOpcUaSensorSource` 중 하나를 생성하도록 분기
- `Config/DefaultGame.ini`에 `DataSourceType=WebSocket` 기본값 + 주석으로 `OpcUaMock` 전환 방법 문서화 — 코드 수정 없이 ini 한 줄만 바꾸면 시뮬레이터 없이도 파이프라인 전체(액터 색상/HUD/알람) 테스트 가능
- 빌드 검증 완료 (UBT 컴파일 성공)

### 본인 작업 (트러블슈팅 포함)
- **인터페이스 스왑 가능성 직접 검증**: `Config/DefaultGame.ini`에서 `DataSourceType=OpcUaMock`으로 직접 바꾸고 파이썬 시뮬레이터 없이 PIE 실행 → 액터/HUD/알람이 전부 정상 동작하는 것을 직접 확인. `ISensorDataSource` 추상화가 "나중에 OPC UA로 교체 가능"하다던 처음 설계 의도가 말뿐이 아니라 실제로 성립한다는 걸 본인이 직접 증명함
- **`OpcUaMock`을 기본값으로 유지하기로 결정**: 빠른 개발 이터레이션을 위해 매번 시뮬레이터를 켜는 과정을 배제하고 싶다고 판단 → `DataSourceType=OpcUaMock`을 커밋 대상 기본값으로 그대로 두기로 결정 (`WebSocket`으로 되돌릴지 물었을 때 본인이 직접 결정)

---
