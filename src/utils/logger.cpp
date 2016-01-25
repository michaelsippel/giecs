/**
 *  src/util/logger.cpp
 *
 *  (C) Copyright 2014 Michael Sippel
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @author Michael Sippel <micha@infinitycoding.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "list.h"
#define _NOSTR_
#include "logger.h"


const char* linfo = "\033[1;49;37mINFO";
const char* lwarning = "\033[1;49;33mWARNING";
const char* lerror = "\033[1;49;31mERROR";


Logger::Logger()
{
    this->log_data = new List<LogData*>();
    this->sub_loggers = new List<Logger*>();
    this->parent = NULL;
}

Logger::Logger(const char *prefix_)
{
    this->generate_prefix_str((char*)prefix_);
    this->log_data = new List<LogData*>();
    this->sub_loggers = new List<Logger*>();
    this->parent = NULL;
}

Logger::Logger(char *prefix_)
{
    this->generate_prefix_str(prefix_);
    this->log_data = new List<LogData*>();
    this->sub_loggers = new List<Logger*>();
    this->parent = NULL;
}

Logger::Logger(Logger *parent_, const char *prefix_)
{
    parent_->sub_loggers->pushBack(parent_);
    this->parent = parent_;

    this->generate_prefix_str((char*)prefix_);
    this->log_data = new List<LogData*>();
    this->sub_loggers = new List<Logger*>();
}

Logger::Logger(Logger *parent_, char *prefix_)
{
    parent_->sub_loggers->pushBack(parent_);
    this->parent = parent_;

    this->generate_prefix_str(prefix_);
    this->log_data = new List<LogData*>();
    this->sub_loggers = new List<Logger*>();
}

Logger::~Logger()
{
}

void Logger::generate_prefix_str(char *prefix_)
{
    strcpy(this->prefix, prefix_);

    if(this->parent != NULL)
    {
        strcpy(this->prefix_str, this->parent->prefix_str);
        strcat(this->prefix_str, "/");
    }
    strcat(this->prefix_str, prefix_);
}

void Logger::log(const char* typestr, const char *text, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, text);
    vsprintf(buffer, text, args);
    va_end(args);

    LogData *data = new LogData(this, typestr, buffer);
    this->log_data->pushBack(data);

    char *time_str = ctime(&data->log_time);
    time_str[strlen(time_str)-1] = '\0';

    printf("[%s %s] %s: %s \033[0m\n", time_str, typestr, this->prefix_str, data->text);
}

LogData::LogData(const char* typestr_, char *text_)
    : typestr(typestr_)
{
    strcpy(this->text, text_);
    this->parent = NULL;
    time(&this->log_time);
}

LogData::LogData(Logger *parent_, const char* typestr_, char *text_)
    : parent(parent_), typestr(typestr_)
{
    strcpy(this->text, text_);
    time(&this->log_time);
}

LogData::~LogData()
{
}

char *LogData::getPrefixString(void)
{
    return (char*) &this->parent->prefix_str;
}

