from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    motor_node = Node(
        name='motor_sys',
        package='motor_control',
        executable='dc_motor',
        emulate_tty=True,
        output='screen',
        parameters=[{
            'sample_time': 0.01,
            'gain_K': 2.16,
            'tau_T': 0.05,
            'initial_conditions': 0.0,
        }]
    )

    setpoint_node = Node(
        name='sp_gen',
        package='motor_control',
        executable='setpoint',
        emulate_tty=True,
        output='screen',
        parameters=[{
            'amplitude': 1.0,
            'omega': 1.0,
            'timer_period': 0.01
        }]
    )

    controller_node = Node(
        name='ctrl',
        package='motor_control',
        executable='controller',
        emulate_tty=True,
        output='screen',
        parameters=[{
            'Kp': 0.1,
            'Kd': 0.0,
            'Ki': 0.0,
            'sample_time': 0.01
        }]
    )

    return LaunchDescription([
        motor_node,
        setpoint_node,
        controller_node
    ])