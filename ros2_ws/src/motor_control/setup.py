from setuptools import find_packages, setup
import os
from glob import glob


package_name = 'motor_control'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        (os.path.join('share', package_name, 'launch'), glob(os.path.join('launch', '*launch.[pxy][yma]*'))),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='',
    maintainer_email='',
    description='',
    license='Apache-2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'dc_motor = motor_control.dc_motor:main',
            'setpoint = motor_control.setpoint:main',
            'controller = motor_control.controller:main'
        ],
    },
)