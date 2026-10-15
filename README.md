# PintOS · WEEK7

Krafton Jungle · **Project 1: Threads**

Alarm Clock · Priority Scheduling · Priority Donation · MLFQS

[팀 진행 보드](https://github.com/users/CatIsApple/projects/6) · [이슈](https://github.com/CatIsApple/PintOS/issues)

## 개발 환경

Docker Desktop, VS Code, VS Code의 **Dev Containers** 확장을 준비합니다.

```bash
git clone https://github.com/CatIsApple/PintOS.git
cd PintOS
code .
```

VS Code 명령 팔레트에서 **Dev Containers: Reopen in Container**를 실행합니다.

컨테이너는 Ubuntu 22.04 / amd64이며 작업 경로는 `/workspaces/PintOS`입니다.

## 빌드와 테스트

컨테이너 터미널에서 빌드와 전체 Threads 테스트를 실행합니다.

```bash
cd /workspaces/PintOS/pintos
source activate
cd threads
make
make check
```

개별 테스트는 같은 `threads/` 디렉토리에서 실행합니다.

```bash
make -C build tests/threads/alarm-zero.result
cat build/tests/threads/alarm-zero.result
```

| 결과 | 경로 |
| --- | --- |
| 전체 요약 | `pintos/threads/build/results` |
| 개별 로그 | `pintos/threads/build/tests/threads/*.output` |
| MLFQS 로그 | `pintos/threads/build/tests/threads/mlfqs/*.output` |

## 참고 자료

- [7주차 LMS](https://jungle-lms.krafton.com/learning/1316)
- [KAIST PintOS Project 1 명세](https://casys-kaist.github.io/pintos-kaist/project1/introduction.html)
