
from pathlib import Path
import subprocess
import json
from concurrent.futures import ThreadPoolExecutor, as_completed

REMOVE_SECONDS = 5
MAX_WORKERS = 10


def process_video(video: Path):

    try:
        # 获取视频时长
        cmd = [
            "ffprobe",
            "-v", "quiet",
            "-print_format", "json",
            "-show_format",
            str(video)
        ]

        result = subprocess.check_output(cmd)
        data = json.loads(result)

        duration = float(data["format"]["duration"])

        keep_time = duration - REMOVE_SECONDS

        if keep_time <= 0:
            print(f"[跳过] 视频太短: {video}")
            return

        out = video.with_stem(video.stem + "_trimmed")

        # ffmpeg裁剪
        subprocess.run([
            "ffmpeg",
            "-y",                 # 覆盖输出
            "-i", str(video),
            "-t", str(keep_time),
            "-c", "copy",
            str(out)
        ], check=True)

        print(f"[完成] {video.name}")

    except Exception as e:
        print(f"[失败] {video.name}: {e}")


videos = list(Path(".").glob("*.mp4"))

with ThreadPoolExecutor(max_workers=MAX_WORKERS) as executor:

    futures = [
        executor.submit(process_video, video)
        for video in videos
    ]

    for future in as_completed(futures):
        future.result()

print("全部处理完成")


# git remote set-url origin git@github.com:Heisenberg-Ambition/AchieveAmbition.git
# git remote set-url origin https://github.com/Heisenberg-Ambition/AchieveAmbition.git