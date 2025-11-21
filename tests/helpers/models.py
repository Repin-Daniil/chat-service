from typing import List
from typing import Optional

from pydantic import BaseModel
from pydantic import Field

from helpers.utils import fake


class User(BaseModel):
    username: str = Field(default_factory=fake.user_name)
    password: str = Field(default_factory=fake.password)
    display_name: Optional[str] = Field(default_factory=fake.name)
    biography: Optional[str] = Field(default_factory=fake.paragraph)


# todo Chat
