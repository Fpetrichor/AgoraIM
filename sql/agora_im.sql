-- =====================================================
-- AgoraIM Database Schema
-- Version : 1.0
-- Author  : petrichor
-- =====================================================

/*
======================================================

User Status
0 Offline
1 Online
2 Busy
3 Invisible

Chat Type
0 Private
1 Group

Message Type
0 Text
1 Image
2 File
3 Voice
4 System

Member Role
0 Member
1 Admin
2 Owner

Recipient Status
0 Pending
1 Delivered
2 Read
3 Failed

======================================================
*/

SET NAMES utf8mb4;

USE agora_im;

DROP TABLE IF EXISTS message_recipients;
DROP TABLE IF EXISTS messages;
DROP TABLE IF EXISTS chat_members;
DROP TABLE IF EXISTS chat_groups;
DROP TABLE IF EXISTS chats;
DROP TABLE IF EXISTS friendships;
DROP TABLE IF EXISTS users;

CREATE TABLE IF NOT EXISTS users (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,

    username VARCHAR(32) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,

    nickname VARCHAR(32) NOT NULL,
    avatar_url VARCHAR(255),

    status TINYINT UNSIGNED NOT NULL DEFAULT 0,

    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,

    updated_at DATETIME NOT NULL
        DEFAULT CURRENT_TIMESTAMP
        ON UPDATE CURRENT_TIMESTAMP,

    UNIQUE KEY uk_username(username)
);

CREATE TABLE IF NOT EXISTS friendships (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,

    user_id BIGINT UNSIGNED NOT NULL,
    friend_id BIGINT UNSIGNED NOT NULL,

    status TINYINT UNSIGNED NOT NULL DEFAULT 0,

    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,

    updated_at DATETIME NOT NULL
        DEFAULT CURRENT_TIMESTAMP
        ON UPDATE CURRENT_TIMESTAMP,

    UNIQUE KEY uk_friendship (user_id, friend_id),

    INDEX idx_user_id (user_id)
);

CREATE TABLE IF NOT EXISTS chats (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,

    chat_type TINYINT UNSIGNED NOT NULL,

    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,

    INDEX idx_chat_type(chat_type)
);

CREATE TABLE IF NOT EXISTS chat_groups (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,

    chat_id BIGINT UNSIGNED NOT NULL,

    group_name VARCHAR(64) NOT NULL,

    owner_id BIGINT UNSIGNED NOT NULL,

    announcement VARCHAR(512),

    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
        ON UPDATE CURRENT_TIMESTAMP,

    UNIQUE KEY uk_chat(chat_id),

    INDEX idx_owner(owner_id)
);

CREATE TABLE IF NOT EXISTS chat_members (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,

    chat_id BIGINT UNSIGNED NOT NULL,

    user_id BIGINT UNSIGNED NOT NULL,

    role TINYINT UNSIGNED NOT NULL DEFAULT 0,

    joined_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,

    UNIQUE KEY uk_chat_member(chat_id, user_id),

    INDEX idx_chat(chat_id),
    INDEX idx_user(user_id)
);

CREATE TABLE IF NOT EXISTS messages (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,

    chat_id BIGINT UNSIGNED NOT NULL,

    sender_id BIGINT UNSIGNED NOT NULL,

    message_type TINYINT UNSIGNED NOT NULL DEFAULT 0,

    content TEXT NOT NULL,

    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,

    INDEX idx_chat(chat_id),

    INDEX idx_sender(sender_id),

    INDEX idx_chat_time(chat_id, created_at)
);

CREATE TABLE IF NOT EXISTS message_recipients (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,

    message_id BIGINT UNSIGNED NOT NULL,

    user_id BIGINT UNSIGNED NOT NULL,

    status TINYINT UNSIGNED NOT NULL DEFAULT 0,

    read_at DATETIME DEFAULT NULL,

    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,

    UNIQUE KEY uk_message_user(message_id, user_id),

    INDEX idx_user(user_id),

    INDEX idx_status(status)
);