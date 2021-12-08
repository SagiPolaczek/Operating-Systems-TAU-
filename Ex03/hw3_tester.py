#!/usr/bin/python3.7
import os
import re
import stat
import subprocess as sp
import sys
from pathlib import Path
from time import sleep
import logging


################################################################################################
def compile_student_files():
    try:
        s = sp.run(["gcc", "-O3", "-Wall", "-std=c11", "message_reader.c", "-o", "message_reader"], check=True)
        os.chmod(f"./message_reader", stat.S_IRWXO | stat.S_IRWXG | stat.S_IRWXU)
        print("reader compile succeed ")
    except Exception as e:
        return 1

    try:
        s = sp.run(["gcc", "-O3", "-Wall", "-std=c11", "message_sender.c", "-o", "message_sender"],
                   check=True)
        os.chmod(f"./message_sender",
                 stat.S_IRWXO | stat.S_IRWXG | stat.S_IRWXU)
        print("sender compile succeed ")
    except Exception as e:
        return 1

    try:
        s = sp.run(["make"], check=True)

        if s.returncode != 0:  # check if compilation works
            print("Make failed")  # DEBUG
            return 1
        print("make succeed ")
    except Exception as e:
        print("make compile failed", e)
        return 1

    try:  # Check if the .ko file was created
        if not os.path.exists("message_slot.ko"):
            print(".ko file missing")  # DEBUG
            return 1
    except Exception as e:
        print("OSError compile_files: ", e)
        return 1

    return 0


#################################################################################################

def read_message(device_path_Name, chID, is_user_file=True):
    try:
        if is_user_file:
            print("Hey2")
            p = sp.run(args=['./message_reader', device_path_Name, str(chID)], capture_output=True, text=True)
            print("p stdout")
            print(p.stdout)
        else:
            print("Hey3")
            p = sp.run(args=['./message_reader_true', device_path_Name, str(chID)], capture_output=True, text=True)
    except sp.SubprocessError as e:
        print("In here")
        raise sp.SubprocessError

    return p.stdout


def send_message(device_path_Name, chID, msgStr, is_user_file=True):
    try:
        if is_user_file:
            p = sp.run(args=['./message_sender', device_path_Name, str(chID), msgStr])
        else:
            p = sp.run(args=['./message_sender_true', device_path_Name, str(chID), msgStr])
    except sp.SubprocessError as e:
        raise sp.SubprocessError


TEST_POINTS_REDUCTION = 2  # change this to whatever would work with the tests
MINOR_POINT_REDUCTION = 1
POINTS_REDUCTION_BUG = 5
POINTS_SENDER_DOESNT_WORK = 10
OVERWRITE_MODE, APPEND_MODE = 0, 1  # Don't change values. these integers are the ones needed.


def message_reader_text_test(stud_logger, file_path_to_exe, dev_name):
    test_errors_str = ""
    points_to_reduct = 0
    minor_num = 250
    try:
        test_output_name = file_path_to_exe + 'outputUserReader.txt'
        with open(test_output_name, 'w') as test_log:  # ./assignments/Yuval Checker_999999999/output1.txt
            # Check If the user's message_reader is valid (Most of them aren't...) :(
            if send_message(file_path_to_exe, dev_name, OVERWRITE_MODE, minor_num, "messageToBeRead", stud_logger,
                            is_user_file=False) == 1:
                test_errors_str += "message_sender doesn't work. "
                points_to_reduct += TEST_POINTS_REDUCTION
            # Read with user's message_reader
            if read_message(True, file_path_to_exe, dev_name, 1, stud_logger) == 1:
                test_errors_str += "message_reader output not as requested. "
                points_to_reduct += TEST_POINTS_REDUCTION
    except OSError as e:
        print("OSError First One: ", e)

    return points_to_reduct, test_errors_str


def run_tests(device_path_name, minor_num):
    points_to_reduct = 0
    test_errors_str = ""
    print("in test 0")
    tests_arguments = [
        {'ch_id': 10, 'message': "10Hello hi", 'minor': minor_num, 'mode': OVERWRITE_MODE, 'expected': '10Hello hi'},
        # ./tests/output0.txt
        {'ch_id': 10, f'message': "10Overwritten", 'minor': minor_num, 'mode': OVERWRITE_MODE,
         'expected': '10Overwritten'},
        # ./tests/output2.txt
        {'ch_id': 20, 'message': "20NiceString", 'minor': minor_num, 'mode': APPEND_MODE, 'expected': '20NiceString'},
        # ./tests/output3.txt
        {'ch_id': 20, 'message': "20OverWritten", 'minor': minor_num, 'mode': APPEND_MODE, 'expected': '20OverWritten'},
        # ./tests/output4.txt
        {'ch_id': 10, 'message': "10AgainOverwritten", 'minor': minor_num, 'mode': OVERWRITE_MODE,
         'expected': '10AgainOverwritten'},
        # ./tests/output5.txt
    ]
    print("in test 1")
    for test_number, test_args in enumerate(tests_arguments):
        try:
            send_message(device_path_name, test_args['ch_id'], test_args['message'], True)
        except sp.SubprocessError as e:
            points_to_reduct += TEST_POINTS_REDUCTION
            test_errors_str += "message_sender failed. "
            continue
        print("in test 2")
        try:
            print("Hey")
            user_output = read_message(device_path_name, test_args['ch_id'], True)
        except sp.SubprocessError as e:
            print("Hey I am here")
            points_to_reduct += POINTS_REDUCTION_BUG
            print("42")
            test_errors_str += "message_reader failed. "
            continue
        print("in test 3")
        expected_output = test_args['expected']
        if user_output == expected_output:
            print(f"test {test_number} succeed\n")
        elif expected_output in user_output:
            points_to_reduct += MINOR_POINT_REDUCTION
            test_errors_str += f"test {test_number} failed. user_output has more characters then expected "
        else:
            points_to_reduct += TEST_POINTS_REDUCTION
            test_errors_str += f"test {test_number} failed. "
    return points_to_reduct, test_errors_str


def remove_char_device(device_path_name):
    try:
        ret = os.system(f'sudo rm -f {device_path_name}')
    except OSError as e:
        # stud_logger.info("remove char device failed", e)
        return 1

    return 0


def load_module():
    try:
        s = sp.run(["sudo", "insmod", "message_slot.ko"], check=True)
        s = sp.run(["sudo", "chmod", "777", "message_slot.ko"], check=True)
        print("load module succeed")
    except sp.SubprocessError as e:
        raise sp.SubprocessError
    return 0


def remove_module(exe_files_path=None, stud_logger=None):
    try:
        s = sp.run(["sudo", "rmmod", "message_slot"], check=True)
        if s.returncode == 1:
            print("rmmod failed for user:", exe_files_path)
            return 1  # if I can't remove the Module, I shouldn't run the other directories until its off
    except sp.SubprocessError as e:
        try:
            stud_logger.info("remove_module failed", e)
        except Exception:
            pass
        return 1

    try:
        stud_logger.info("remove_module success")
    except Exception:
        pass
    return 0


def load_device(device_path_name, MINOR_NUMBER):
    try:
        s = sp.run(["sudo", "mknod", device_path_name, "c", str(240), str(MINOR_NUMBER)], check=True)
        s = sp.run(["sudo", "chmod", "777", device_path_name], check=True)
        print("load device succeed")
    except sp.SubprocessError as e:
        raise sp.SubprocessError

    return device_path_name, MINOR_NUMBER


def clean_tests_env(device_path_name):
    remove_char_device(device_path_name)
    try:
        remove_module()
    except Exception as e:
        print("clean test env failed ", e)
    print("finish clean test env")


def run(device_name,minor_num):
    try:
        points_to_reduct, test_errors_str = run_tests(device_name, minor_num)
        student_GRADE = 100 - points_to_reduct
        print("grade = " + str(student_GRADE))
        print("test_errors_str= " + test_errors_str)
    except Exception as e:
        print(e)
        print("error in the end")



def main():
    device_name = "/dev/tester"
    print("=========================start remove files from the past ========================")
    clean_tests_env(device_name)
    print("=========================start compile ========================")
    if compile_student_files() != 0:
        print("problem compile")
    print("=========================start load cleanly ========================")
    load_module()
    MINOR_NUMBER = 20
    load_device(device_name, MINOR_NUMBER)
    print("=========================start run the tests ========================")
    run(device_name,MINOR_NUMBER)
    print("============================= finish ========================")
main()
