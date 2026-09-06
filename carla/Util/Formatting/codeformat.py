#!/usr/bin/python

#
# Copyright (c) 2017-2020 Intel Corporation
#
# Helper script for code formatting using clang-format-3.9 and autopep

import argparse
import filecmp
import os
import re
import sets
import subprocess
import sys
from termcolor import cprint

SCRIPT_VERSION = "1.3"


class CodeFormatter:

    def __init__(self, command, expectedVersion, formatCommandArguments, verifyCommandArguments, verifyOutputIsDiff, fileEndings, fileDescription, installCommand):
        self.command = command
        self.expectedVersion = expectedVersion
        self.formatCommandArguments = formatCommandArguments
        self.verifyCommandArguments = verifyCommandArguments
        self.verifyOutputIsDiff = verifyOutputIsDiff
        self.fileEndings = fileEndings
        self.fileDescription = fileDescription
        self.installCommand = installCommand

    def verifyFormatterVersion(self):
        try:
            versionOutput = subprocess.check_output([self.command, "--version"]).rstrip('\r\n')
            if self.expectedVersion != "":
                if versionOutput.startswith(self.expectedVersion):
                    print("[OK] Found formatter '" + versionOutput + "'")
                    return
                else:
                    cprint("[NOT OK] Found '" + versionOutput + "'", "red")
                    cprint("[NOT OK] Version string does not start with '" + self.expectedVersion + "'", "red")
            else:
                return
        except:
            cprint("[ERROR] Could not run " + self.command, "red")
            cprint("[INFO] Please check if correct version is installed or install with '" +
                   self.installCommand + "'", "blue")
        sys.exit(1)

    def printInputFiles(self):
        if len(self.inputFiles) > 0:
            print("Found " + self.fileDescription + " files:")
            for fileName in self.inputFiles:
                print(fileName)
            print("")

    def formatFile(self, fileName):
        commandList = [self.command]
        commandList.extend(self.formatCommandArguments)
        commandList.append(fileName)
        try:
            subprocess.check_output(commandList, stderr=subprocess.STDOUT)
            print("[OK] " + fileName)
        except subprocess.CalledProcessError as e:
            cprint("[ERROR] " + fileName + " (" + e.output.rstrip('\r\n') + ")", "red")
            return True
        return False

    def performGitDiff(self, fileName, verifyOutput):
        try:
            diffProcess = subprocess.Popen(
                ["git", "diff", "--color=always", "--exit-code", "--no-index", "--", fileName, "-"],
                                           stdin=subprocess.PIPE,
                                           stdout=subprocess.PIPE,
                                           stderr=subprocess.PIPE)
            diffOutput, _ = diffProcess.communicate(verifyOutput)
            if diffProcess.returncode == 0:
                diffOutput = ""
        except OSError:
            cprint("[ERROR] Failed to run git diff on " + fileName, "red")
            return (True, "")
        return (False, diffOutput)

    def verifyFile(self, fileName, printDiff):
        commandList = [self.command]
        commandList.extend(self.verifyCommandArguments)
        commandList.append(fileName)
        try:
            verifyOutput = subprocess.check_output(commandList, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            cprint("[ERROR] " + fileName + " (" + e.output.rstrip('\r\n') + ")", "red")
            return True

        diffOutput = ""
        if self.verifyOutputIsDiff:
            diffOutput = verifyOutput
        else:
            status, diffOutput = self.performGitDiff(fileName, verifyOutput)
            if status:
                return True

        if diffOutput != "":
            cprint("[NOT OK] " + fileName, "red")
            if printDiff:
                print(diffOutput.rstrip('\r\n'))
            return True

        print("[OK] " + fileName)
        return False


class CodeFormatterClang(CodeFormatter):
    CLANG_FORMAT_FILE = ".clang-format"
    CHECKED_IN_CLANG_FORMAT_FILE = "clang-format"
    CODE_FORMAT_IGNORE_FILE = ".codeformatignore"

    def __init__(self):
        CodeFormatter.__init__(self,
                               command="clang-format-3.9",
                               expectedVersion="clang-format version 3.9",
                               formatCommandArguments=["-style=file", "-fallback-style=none", "-i"],
                               verifyCommandArguments=["-style=file", "-fallback-style=none"],
                               verifyOutputIsDiff=False,
                               fileEndings=["cpp", "hpp", "c", "h", "cc"],
                               fileDescription="source and header",
                               installCommand="sudo apt-get install clang-format-3.9")
        self.scriptPath = os.path.dirname(os.path.abspath(__file__))
        self.checkedInClangFormatFile = os.path.join(self.scriptPath, CodeFormatterClang.CHECKED_IN_CLANG_FORMAT_FILE)

    def verifyFormatterVersion(self):
        CodeFormatter.verifyFormatterVersion(self)
        self.verifyClangFormatFileExistsAndMatchesCheckedIn()

    def verifyCheckedInClangFormatFileExists(self):
        if os.path.exists(self.checkedInClangFormatFile):
            print("[OK] Found " + CodeFormatterClang.CHECKED_IN_CLANG_FORMAT_FILE + " file (the one that should be in a repository) " +
                  self.checkedInClangFormatFile)
        else:
            cprint("[WARN] Not found " + CodeFormatterClang.CHECKED_IN_CLANG_FORMAT_FILE + " file " +
                   self.checkedInClangFormatFile, "yellow")
            self.confirmWithUserClangFormatFileCantBeVerified()

    def confirmWithUserClangFormatFileCantBeVerified(self):
        if not self.args.yes:
            answer = raw_input("Are you sure your .clang-format file is up-to-date and you want to continue? (y/N)")
            if answer != "y":
                sys.exit(1)

    def verifyClangFormatFileExistsAndMatchesCheckedIn(self):
        self.verifyCheckedInClangFormatFileExists()
        foundClangFormatFiles = sets.Set()
        for fileName in self.inputFiles:
            dirName = os.path.dirname(os.path.abspath(fileName))
            if not self.findClangFormatFileStartingFrom(dirName, fileName, foundClangFormatFiles):
                sys.exit(1)

    def findClangFormatFileStartingFrom(self, dirName, fileName, foundClangFormatFiles):
        clangFormatFile = os.path.join(dirName, CodeFormatterClang.CLANG_FORMAT_FILE)
        if os.path.exists(clangFormatFile):
            if clangFormatFile not in foundClangFormatFiles:
                foundClangFormatFiles.add(clangFormatFile)
                if os.path.exists(self.checkedInClangFormatFile) and \
                   not filecmp.cmp(self.checkedInClangFormatFile, clangFormatFile):
                    cprint("[WARN] " + clangFormatFile + " does not match " + self.checkedInClangFormatFile, "yellow")
                    self.confirmWithUserClangFormatFileCantBeVerified()
                else:
                    print("[OK] Found " + CodeFormatterClang.CLANG_FORMAT_FILE +
                          " file (used by the formatter) " + clangFormatFile)
            return True
        else:
            dirNameOneLevelUp = os.path.dirname(dirName)
            if dirNameOneLevelUp == dirName:
                # dirName was already root folder -> clang-format file not found
                cprint("[ERROR] Not found " + CodeFormatterClang.CLANG_FORMAT_FILE + " for " +
                       fileName + " in same directory or in any parent directory", "red")
                return False
            else:
                return self.findClangFormatFileStartingFrom(dirNameOneLevelUp, fileName, foundClangFormatFiles)


class CodeFormatterAutopep(CodeFormatter):

    def __init__(self):
        CodeFormatter.__init__(self,
                               command="autopep8",
                               expectedVersion="",
                               formatCommandArguments=["--in-place", "--max-line-length=119"],
                               verifyCommandArguments=["--diff", "--max-line-length=119"],
                               verifyOutputIsDiff=True,
                               fileEndings=["py"],
                               fileDescription="python",
                               installCommand="sudo apt-get install python-pep8 python-autopep8")


class CodeFormat:

    def __init__(self):
        self.failure = False
        self.codeFormatterInstances = []
        return

    def parseCommandLine(self):
        parser = argparse.ArgumentParser(
            description="Helper script for code formatting.")
        parser.add_argument("input", nargs="+",
                            help="files or directories to process")
        parser.add_argument("-v", "--verify", action="store_true",
                            help="do not change file, but only verify the format is correct")
        parser.add_argument("-d", "--diff", action="store_true",
                            help="show diff, implies verify mode")
        parser.add_argument("-e", "--exclude", nargs="+", metavar="exclude",
                            help="exclude files or directories containing words from the exclude list in their names")
        parser.add_argument("-y", "--yes", action="store_true",
                            help="do not ask for confirmation before formatting more than one file")
        parser.add_argument("--version", action="version", version="%(prog)s " + SCRIPT_VERSION)
        self.args = parser.parse_args()
        if self.args.diff:
            self.args.verify = True

    def addCodeFormatter(self, codeFormatterInstance):
        self.codeFormatterInstances.append(codeFormatterInstance)

    def scanForInputFiles(self):
        for formatterInstance in self.codeFormatterInstances:
            filePattern = re.compile("^[^.].*\.(" + "|".join(formatterInstance.fileEndings) + ")$")
            formatterInstance.inputFiles = []
            for fileOrDirectory in self.args.input:
                if os.path.exists(fileOrDirectory):
                    formatterInstance.inputFiles.extend(self.scanFileOrDirectory(fileOrDirectory, filePattern))
                else:
                    cprint("[WARN] Cannot find '" + fileOrDirectory + "'", "yellow")

    def scanFileOrDirectory(self, fileOrDirectory, filePattern):
        fileList = []
        if os.path.isdir(fileOrDirectory):
            for root, directories, fileNames in os.walk(fileOrDirectory):
                directories[:] = self.filterDirectories(root, directories)
                for filename in filter(lambda name: filePattern.match(name), fileNames):
                    fullFilename = os.path.join(root, filename)
                    if self.isFileNotExcluded(fullFilename):
                        fileList.append(fullFilename)
        else:
            if self.isFileNotExcluded(fileOrDirectory) and (filePattern.match(os.path.basename(fileOrDirectory)) is not None):
                fileList.append(fileOrDirectory)
        return fileList

    def filterDirectories(self, root, directories):
        # Exclude hidden directories and all directories that have a CODE_FORMAT_IGNORE_FILE
        directories[:] = [directory for directory in directories if
                          not directory.startswith(".") and
                          not os.path.exists(os.path.join(root, directory, CodeFormatterClang.CODE_FORMAT_IGNORE_FILE))]
        return directories

    def isFileNotExcluded(self, fileName):
        if self.args.exclude is not None:
            for excluded in self.args.exclude:
                if excluded in fileName:
                    return False
        if os.path.islink(fileName):
            return False

        return True

    def confirmWithUserFileIsOutsideGit(self, fileName):
        cprint("[WARN] File is not in a Git repo: " + fileName, "yellow")
        answer = raw_input("Are you sure to code format it anyway? (y/Q)")
        if answer != "y":
            sys.exit(1)

    def confirmWithUserFileIsUntracked(self, fileName):
        cprint("[WARN] File is untracked in Git: " + fileName, "yellow")
        answer = raw_input("Are you sure to code format it anyway? (y/Q)")
        if answer != "y":
            sys.exit(1)

    def confirmWithUserGitRepoIsNotClean(self, gitRepo):
        cprint("[WARN] Git repo is not clean: " + gitRepo, "yellow")
        answer = raw_input("Are you sure to code format files in it anyway? (y/Q)")
        if answer != "y":
            sys.exit(1)

    def checkInputFilesAreInCleanGitReposAndAreTracked(self):
        if self.args.verify or self.args.yes:
            return
        gitRepos = sets.Set()
        for formatterInstance in self.codeFormatterInstances:
            for fileName in formatterInstance.inputFiles:
                gitRepo = self.getGitRepoForFile(fileName)
                if gitRepo is None:
                    self.confirmWithUserFileIsOutsideGit(fileName)
                else:
                    self.gitUpdateIndexRefresh(gitRepo)
                    if not self.isTrackedFile(fileName):
                        self.confirmWithUserFileIsUntracked(fileName)
                    elif gitRepo not in gitRepos:
                        gitRepos.add(gitRepo)
                        if not self.isCleanGitRepo(gitRepo):
                            self.confirmWithUserGitRepoIsNotClean(gitRepo)

    def getGitRepoForFile(self, fileName):
        if not self.isInsideGitRepo(fileName):
            return None
        try:
            gitProcess = subprocess.Popen(["git", "rev-parse", "--show-toplevel"],
                                          stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE,
                                          cwd=os.path.dirname(fileName))
            gitOutput, _ = gitProcess.communicate()
            if gitProcess.returncode == 0:
                return gitOutput.rstrip('\r\n')
        except OSError:
            cprint("[ERROR] Failed to run 'git rev-parse --show-toplevel' for " + fileName, "red")
        return None

    def isInsideGitRepo(self, fileName):
        try:
            gitProcess = subprocess.Popen(["git", "rev-parse", "--is-inside-work-tree"],
                                          stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE,
                                          cwd=os.path.dirname(fileName))
            gitOutput, _ = gitProcess.communicate()
            if gitProcess.returncode == 0:
                return gitOutput.rstrip('\r\n') == "true"
        except OSError:
            cprint("[ERROR] Failed to run 'git rev-parse --is-inside-work-tree' for " + fileName, "red")
        return False

    def isTrackedFile(self, fileName):
        try:
            gitProcess = subprocess.Popen(["git", "ls-files", "--error-unmatch", "--", os.path.basename(fileName)],
                                          stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE,
                                          cwd=os.path.dirname(fileName))
            _, _ = gitProcess.communicate()
            if gitProcess.returncode == 0:
                return True
        except OSError:
            cprint("[ERROR] Failed to run 'git ls-files --error-unmatch' for " + fileName, "red")
        return False

    def isCleanGitRepo(self, gitRepo):
        try:
            gitProcess = subprocess.Popen(["git", "diff-index", "--quiet", "HEAD", "--"],
                                          stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE,
                                          cwd=gitRepo)
            _, _ = gitProcess.communicate()
            if gitProcess.returncode == 0:
                return True
        except OSError:
            cprint("[ERROR] Failed to run 'git diff-index --quiet HEAD --' for " + gitRepo, "red")
        return False

    def gitUpdateIndexRefresh(self, gitRepo):
        try:
            gitProcess = subprocess.Popen(["git", "update-index", "-q", "--ignore-submodules", "--refresh"],
                                          stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE,
                                          cwd=gitRepo)
            _, _ = gitProcess.communicate()
            if gitProcess.returncode == 0:
                return True
        except OSError:
            cprint("[ERROR] Failed to run 'git update-index -q --ignore-submodules --refresh' for " + gitRepo, "red")
        return False

    def verifyFormatterVersion(self):
        for formatterInstance in self.codeFormatterInstances:
            if len(formatterInstance.inputFiles) > 0:
                formatterInstance.verifyFormatterVersion()

    def printMode(self):
        if self.args.verify:
            cprint("VERIFY MODE", attrs=["bold"])
        else:
            cprint("FORMAT MODE", attrs=["bold"])

    def processFiles(self):
        for formatterInstance in self.codeFormatterInstances:
            for fileName in formatterInstance.inputFiles:
                if self.args.verify:
                    self.failure |= formatterInstance.verifyFile(fileName, self.args.diff)
                else:
                    self.failure |= formatterInstance.formatFile(fileName)

    def numberOfInputFiles(self):
        count = 0
        for formatterInstance in self.codeFormatterInstances:
            count += len(formatterInstance.inputFiles)
        return count

    def confirmWithUser(self):
        if self.numberOfInputFiles() == 0:
            cprint("[WARN] No input files (or file endings unknown)", "yellow")
        elif (not self.args.verify) and (not self.args.yes) and self.numberOfInputFiles() > 1:
            for formatterInstance in self.codeFormatterInstances:
                formatterInstance.printInputFiles()
            answer = raw_input("Are you sure to code format " + str(self.numberOfInputFiles()) + " files? (y/N)")
            if answer != "y":
                sys.exit(1)


def main():
    codeFormat = CodeFormat()
    codeFormat.parseCommandLine()
    codeFormat.printMode()

    codeFormat.addCodeFormatter(CodeFormatterClang())
    codeFormat.addCodeFormatter(CodeFormatterAutopep())

    codeFormat.scanForInputFiles()
    codeFormat.verifyFormatterVersion()
    codeFormat.confirmWithUser()
    codeFormat.checkInputFilesAreInCleanGitReposAndAreTracked()
    codeFormat.processFiles()
    if codeFormat.failure:
        cprint("FAILURE", "red")
        sys.exit(1)
    else:
        cprint("SUCCESS", "green")
        sys.exit(0)

if __name__ == "__main__":
    main()
