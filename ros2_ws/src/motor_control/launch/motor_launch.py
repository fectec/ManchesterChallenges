from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    # DC Motor node
    motor_node = Node(
        name='motor_sys',
        package='motor_control',
        executable='dc_motor',
        emulate_tty=True,
        output='screen',
        parameters=[{
            'sys_sample_time': 0.01,
            'sys_gain_K': 2.16,
            'sys_tau_T': 0.05,
            'sys_initial_conditions': 0.0,
        }]
    )

    # Setpoint generator node
    setpoint_node = Node(
        name='sp_gen',
        package='motor_control',
        executable='setpoint',
        emulate_tty=True,
        output='screen',
        parameters=[{
            'amplitude': 2.0,
            'omega': 1.0,
        }]
    )

    # Controller node
    controller_node = Node(
        name='ctrl',
        package='motor_control',
        executable='controller',
        emulate_tty=True,
        output='screen',
        parameters=[{
            'Kp': 0.5,
            'Kd': 0.5,
            'Ki': 0.5,
            'sample_time': 0.01
        }]
    )

    # Create and return launch description
    return LaunchDescription([
        motor_node,
        setpoint_node,
        controller_node
    ])