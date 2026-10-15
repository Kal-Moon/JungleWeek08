# PintOS · WEEK8

Krafton Jungle · **Project 2: User Programs**

Argument Passing · User Memory Access · System Calls · Process Termination Messages · Deny Write on Executables

[팀 진행 보드](https://github.com/users/cocham/projects/24/views/1) · [이슈](https://github.com/cocham/JungleWeek08/issues)

## 개발 환경

Docker Desktop, VS Code, VS Code의 **Dev Containers** 확장을 준비합니다.

```bash
git clone https://github.com/cocham/JungleWeek08.git
cd JungleWeek08
code .
```

VS Code 명령 팔레트에서 **Dev Containers: Reopen in Container**를 실행합니다.

컨테이너는 Ubuntu 22.04 / amd64이며 작업 경로는 `/workspaces/JungleWeek08`입니다.

## 빌드와 테스트

컨테이너 터미널에서 빌드와 전체 User Programs 테스트를 실행합니다.

```bash
cd /workspaces/JungleWeek08/pintos
source activate
cd userprog
make
make check
```

개별 테스트는 같은 `userprog/` 디렉토리에서 실행합니다.

```bash
make -C build tests/userprog/args-single.result
cat build/tests/userprog/args-single.result
```

| 결과 | 경로 |
| --- | --- |
| 전체 요약 | `pintos/userprog/build/results` |
| 개별 로그 | `pintos/userprog/build/tests/userprog/*.output` |
| 파일시스템 기본 로그 | `pintos/userprog/build/tests/filesys/base/*.output` |

## 참고 자료

- [KAIST PintOS Project 2 명세](https://casys-kaist.github.io/pintos-kaist/project2/introduction.html)
