import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_RUNNING,
    DEVICE_CLASS_HEAT,
    ICON_FLOWER,
    ICON_RADIATOR,
    ICON_WEATHER_WINDY
)

from . import CONF_ELECTROLUX_DRYER_MACS_ID, ElectroluxDryerMacsComponent

CONF_POWERED_ON = "powered_on"
CONF_RUNNING = "running"
CONF_DELICATE_MODE = "delicate_mode"
CONF_HEATING = "heating"

TYPES = [
    CONF_POWERED_ON,
    CONF_RUNNING,
    CONF_DELICATE_MODE,
    CONF_HEATING
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ELECTROLUX_DRYER_MACS_ID): cv.use_id(ElectroluxDryerMacsComponent),
        cv.Optional(CONF_POWERED_ON): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_POWER
        ),
        cv.Optional(CONF_RUNNING): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_RUNNING,
            icon=ICON_WEATHER_WINDY
        ),
        cv.Optional(CONF_DELICATE_MODE): binary_sensor.binary_sensor_schema(
            icon=ICON_FLOWER
        ),
        cv.Optional(CONF_HEATING): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_HEAT, 
            icon=ICON_RADIATOR
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        var = await binary_sensor.new_binary_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_binary_sensor")(var))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ELECTROLUX_DRYER_MACS_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)
