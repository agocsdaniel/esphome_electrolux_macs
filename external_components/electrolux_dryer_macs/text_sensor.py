import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv

from . import CONF_ELECTROLUX_DRYER_MACS_ID, ElectroluxDryerMacsComponent

CONF_SELECTED_PROGRAM_NAME = "selected_program_name"

TYPES = [
    CONF_SELECTED_PROGRAM_NAME
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ELECTROLUX_DRYER_MACS_ID): cv.use_id(ElectroluxDryerMacsComponent),
        cv.Optional(CONF_SELECTED_PROGRAM_NAME): text_sensor.text_sensor_schema(),
    }
).extend(cv.COMPONENT_SCHEMA)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        sens = await text_sensor.new_text_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ELECTROLUX_DRYER_MACS_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)

