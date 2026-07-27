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

### 본인 작업 (트러블슈팅 포함)
- (착수 전 — PIE 확인 및 이슈 발생 시 이 섹션에 채워질 예정)

---
