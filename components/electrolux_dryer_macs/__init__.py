from esphome import pins
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_RECEIVE_TIMEOUT, CONF_UART_ID

CODEOWNERS = ["@agocsdaniel"]

MULTI_CONF = True

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor", "sensor", "text_sensor"]

CONF_ELECTROLUX_DRYER_MACS_ID = "electrolux_dryer_macs_id"

CONF_CRC_CHECK = "crc_check"

electrolux_dryer_macs_ns = cg.esphome_ns.namespace("electrolux_dryer_macs")
ElectroluxDryerMacsComponent = electrolux_dryer_macs_ns.class_(
    "ElectroluxDryerMacsComponent", cg.Component, uart.UARTDevice
)


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ElectroluxDryerMacsComponent),
            cv.Optional(CONF_CRC_CHECK, default=False): cv.boolean,
            cv.Optional(CONF_RECEIVE_TIMEOUT, default="200ms"): cv.positive_time_period_milliseconds,
        }
    ).extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_component)
    cg.add(var.set_receive_timeout(config[CONF_RECEIVE_TIMEOUT].total_milliseconds))
    cg.add(var.set_crc_check(config[CONF_CRC_CHECK]))
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

