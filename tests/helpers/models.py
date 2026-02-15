from typing import List
from typing import Optional

from pydantic import BaseModel, Field
from helpers.utils import *


class User(BaseModel):
    username: str = Field(default_factory=username_generator)
    password: str = Field(default_factory=password_generator)
    display_name: Optional[str] = Field(default_factory=display_name_generator)
    biography: Optional[str] = Field(default_factory=biography_generator)
    token: Optional[str] = Field(default=None, exclude=True)
    session_id: Optional[str] = Field(default=None, exclude=True)


class Message(BaseModel):
    chat_id: str = Field(default="")
    payload: str = Field(default_factory=fake.paragraph)
    sender: str = Field(default=None, exclude=True)


class PrivateChat(BaseModel):
    target_username: str = Field(default="")
