#!/usr/bin/env python3

import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ISP_HEADER = ROOT / "include" / "vizionsdk_ros2" / "isp_controls.hpp"
LAUNCH_FILE = ROOT / "launch" / "vizionsdk_camera.launch.py"


def read_isp_parameter_names():
    text = ISP_HEADER.read_text(encoding="utf-8")
    return re.findall(r'\{"(isp\.[a-z0-9_]+)"\s*,', text)


def main():
    launch_text = LAUNCH_FILE.read_text(encoding="utf-8")
    launch_parameters_match = re.search(
        r"ISP_CONTROL_PARAMETERS = \((?P<body>.*?)\)",
        launch_text,
        re.DOTALL,
    )
    assert launch_parameters_match is not None, "launch file must define ISP_CONTROL_PARAMETERS"

    launch_parameters = re.findall(r'"(isp\.[a-z0-9_]+)"', launch_parameters_match.group("body"))
    assert launch_parameters == read_isp_parameter_names()

    assert 'return parameter_name.replace(".", "_")' in launch_text
    assert "for parameter_name in ISP_CONTROL_PARAMETERS" in launch_text
    assert "DeclareLaunchArgument(" in launch_text
    assert "parameter_name: ParameterValue(" in launch_text
    assert '"imu_self_test_mode"' in launch_text
    assert '"imu_self_test_mode": LaunchConfiguration("imu_self_test_mode")' in launch_text
    assert '("namespace", "", "ROS namespace for this camera.")' in launch_text
    assert 'namespace=LaunchConfiguration("namespace")' in launch_text
    assert "enable_cam1" not in launch_text
    assert "cam1_device_index" not in launch_text
    assert "description=" in launch_text


if __name__ == "__main__":
    main()
