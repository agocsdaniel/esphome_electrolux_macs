import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_TIMESTAMP,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

from . import CONF_ELECTROLUX_WASHING_MACHINE_MACS_ID, ElectroluxWashingMachineMacsComponent

CONF_ACTIVE_ALARM = "active_alarm"
CONF_PHASE = "phase"
CONF_DRYING_MODE = "drying_mode"
CONF_PROGRAM_START = "program_start"
CONF_ESTIMATED_END = "estimated_end"
CONF_WASH_START = "wash_start"

TYPES = [
    CONF_ACTIVE_ALARM,
    CONF_PHASE,
    CONF_DRYING_MODE,
    CONF_PROGRAM_START,
    CONF_ESTIMATED_END,
    CONF_WASH_START,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ELECTROLUX_WASHING_MACHINE_MACS_ID): cv.use_id(ElectroluxWashingMachineMacsComponent),
        cv.Optional(CONF_PHASE): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_DRYING_MODE): text_sensor.text_sensor_schema(),
        # Program tracking: need time_id on the hub
        cv.Optional(CONF_PROGRAM_START): text_sensor.text_sensor_schema(device_class=DEVICE_CLASS_TIMESTAMP),
        cv.Optional(CONF_ESTIMATED_END): text_sensor.text_sensor_schema(device_class=DEVICE_CLASS_TIMESTAMP),
        # When a delayed start will begin washing
        cv.Optional(CONF_WASH_START): text_sensor.text_sensor_schema(device_class=DEVICE_CLASS_TIMESTAMP),
        cv.Optional(CONF_ACTIVE_ALARM): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        )
    }
).extend(cv.COMPONENT_SCHEMA)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        sens = await text_sensor.new_text_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ELECTROLUX_WASHING_MACHINE_MACS_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)

