#!/usr/bin/env python2
# encoding: utf-8
# 
# Copyright 2014 Kevin Bader.
# Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
# 
from __future__ import print_function

import argparse
import datetime as DT
import os
import Queue as Q
import subprocess
import sys
import threading


class LogfileAppender(object):

    def __init__(self, log_fn):
        self._lock = threading.Lock()
        self._log_fn = log_fn

    def write(self, msg):
        with self._lock:
            with open(self._log_fn, 'a') as f:
                print(msg, file=f)


def run_job(command, log):
    devnull = open(os.devnull, 'wb')
    start = DT.datetime.now()
    p = subprocess.Popen(command, shell=True, stdout=devnull)
    p.wait()
    end = DT.datetime.now()
    log.write("|".join([command,
                        str(p.returncode),
                        str((end - start).seconds)]))
    if p.returncode != 0:
        print("WARN non-zero exit code for %s: %s" % (command, p.returncode))


def executor(jobQueue, termination_flag, log):
    threadId = threading.current_thread().ident
    # #Python3:
    # threadId = threading.get_ident()
    print("* -> %s" % threadId)
    try:
        while not termination_flag[0]:
            job = jobQueue.get_nowait()
            print("[%s] %s" % (threadId, job))
            run_job(job, log)
    except Q.Empty:
        pass
    print("%s -> ." % threadId)


def main():
    parser = argparse.ArgumentParser(description='Parallel execution of shell commands.')
    parser.add_argument('-j', '--concurrency', metavar='N', type=int, default=32,
                        help='maximal number of concurrent jobs')
    parser.add_argument('--log', default="executor.log", help='log file for command-returncode pairs')
    parser.add_argument('jobfile', metavar='file', type=argparse.FileType('r'),
                        help='newline separated list of jobs to run; given either as filename or with "-" on stdin.')
    args = parser.parse_args()
    jobs = [x.strip() for x in args.jobfile.readlines()]
    args.jobfile.close()
    jobQueue = Q.Queue()
    for job in jobs:
        jobQueue.put(job)
    print("starting %s threads..." % args.concurrency)
    threads = []
    thread_termination_flags = []
    log = LogfileAppender(args.log)

    def update_concurrency(n):
        print("please wait while the concurrency is updated to %s..." % n)
        if len(threads) < n:
            for i in range(n - len(threads)):
                termination_flag = [False]
                t = threading.Thread(target=executor, args=(jobQueue, termination_flag, log,))
                t.start()
                threads.append(t)
                thread_termination_flags.append(termination_flag)
        elif len(threads) > n:
            for i in range(len(threads) - 1, n - 1, -1):
                thread_termination_flags[i][0] = True
            for i in range(len(threads) - 1, n - 1, -1):
                threads[i].join()
                del threads[i]
                del thread_termination_flags[i]
        print("concurrency update done, now running a maximum of %s concurrent executions" % len(threads))

    update_concurrency(args.concurrency)
    if args.jobfile.name != "<stdin>":
        command = ""
        while command != "quit":
            command = sys.stdin.readline().strip()
            if command.startswith("j "):
                try:
                    n = int(command[2:])
                    update_concurrency(n)
                except ValueError:
                    print("failed. usage: set concurrency to <num> with 'j <num>'")
            elif command == 's':
                qsize = jobQueue.qsize()
                nJobs = len(jobs)
                print("processed %s of %s jobs (%s %%), %s jobs remaining" % (nJobs - qsize, nJobs, 100 * (nJobs - qsize) / nJobs, qsize))
            elif command != "quit":
                print("one of\n  'j <num>', where <num> is the number of threads\n  's' for status\n  'quit'")
            if command != "quit" and jobQueue.empty():
                print("JOB QUEUE IS EMPTY, YOU MAY quit NOW :)")
        # TODO well we could also kill currently running jobs...
        try:
            while True:
                print("not executing %s!" % jobQueue.get_nowait())
        except Q.Empty:
            pass
    for thread in threads:
        thread.join()


if __name__ == '__main__':
    main()
